#include "database.h"

#include "board/abstractboard.h"
#include "cache.h"
#include "captcha/abstractcaptchaengine.h"
#include "controller/controller.h"
#include "search.h"
#include "settingslocker.h"
#include "stored/banneduser.h"
#include "stored/banneduser-odb.hxx"
#include "stored/postcounter.h"
#include "stored/postcounter-odb.hxx"
#include "stored/registereduser.h"
#include "stored/registereduser-odb.hxx"
#include "stored/thread.h"
#include "stored/thread-odb.hxx"
#include "tools.h"
#include "transaction.h"
#include "translator.h"

#include <BDirTools>
#include <BeQt>
#include <BSqlDatabase>
#include <BSqlQuery>
#include <BSqlResult>
#include <BSqlWhere>
#include <BTerminal>
#include <BTextTools>
#include <BUuid>

#include <QByteArray>
#include <QCryptographicHash>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QDomCDATASection>
#include <QDomDocument>
#include <QDomElement>
#include <QDomNode>
#include <QDomProcessingInstruction>
#include <QDomText>
#include <QFile>
#include <QFileInfo>
#include <QReadLocker>
#include <QReadWriteLock>
#include <QScopedPointer>
#include <QSettings>
#include <QSharedPointer>
#include <QString>
#include <QStringList>
#include <QTimeZone>
#include <QVariant>
#include <QVariantMap>
#include <QWriteLocker>

#include <cppcms/http_request.h>

#include <odb/connection.hxx>
#include <odb/database.hxx>
#include <odb/query.hxx>
#include <odb/schema-catalog.hxx>
#include <odb/sqlite/database.hxx>
#include <odb/transaction.hxx>

B_DECLARE_TRANSLATE_FUNCTION

namespace Database
{

struct PostTmpInfo
{
    RefMap refs;
    QString board;
    QString text;
};

RefKey::RefKey()
{
    postNumber = 0;
}

RefKey::RefKey(const QString &board, quint64 post)
{
    boardName = board;
    postNumber = post;
}

bool RefKey::isValid() const
{
    return !boardName.isEmpty() && postNumber;
}

bool RefKey::operator <(const RefKey &other) const
{
    return boardName < other.boardName || (boardName == other.boardName && postNumber < other.postNumber);
}

CreatePostParameters::CreatePostParameters(const cppcms::http::request &req, const QMap<QString, QString> &ps,
                                           const QList<Tools::File> &fs, const QLocale &l) :
    files(fs), locale(l), params(ps), request(req)
{
    bumpLimit = 0;
    postLimit = 0;
    error = 0;
    description = 0;
}

CreateThreadParameters::CreateThreadParameters(const cppcms::http::request &req, const QMap<QString, QString> &ps,
                                               const QList<Tools::File> &fs, const QLocale &l) :
    files(fs), locale(l), params(ps), request(req)
{
    threadLimit = 0;
    error = 0;
    description = 0;
}

EditPostParameters::EditPostParameters(const cppcms::http::request &req, const QString &board, quint64 post) :
    boardName(board), postNumber(post), request(req)
{
    draft = false;
    error = 0;
    raw = false;
}

class CreatePostInternalParameters
{
public:
    const cppcms::http::request &request;
    const Tools::PostParameters &params;
    const Tools::FileList &files;
    const QLocale &locale;
public:
    AbstractBoard::FileTransaction fileTransaction;
    unsigned int bumpLimit;
    unsigned int postLimit;
    QString *error;
    QString *description;
    QDateTime dateTime;
    quint64 threadNumber;
    quint64 *postNumber;
    RefMap *referencedPosts;
    RefMap refs;
    QString processedText;
public:
    explicit CreatePostInternalParameters(const cppcms::http::request &req, const Tools::PostParameters &ps,
                                          const Tools::FileList &fs, AbstractBoard *board,
                                          const QLocale &l = BCoreApplication::locale()) :
        request(req), params(ps), files(fs), locale(l), fileTransaction(board)
    {
        error = 0;
        description = 0;
        referencedPosts = 0;
        threadNumber = 0;
        postNumber = 0;
        if (board)
            processedText = Controller::processPostText(params.value("text"), board->name(), &refs);
    }
    explicit CreatePostInternalParameters(CreatePostParameters &p, AbstractBoard *board) :
        request(p.request), params(p.params), files(p.files), locale(p.locale), fileTransaction(board)
    {
        bumpLimit = p.bumpLimit;
        postLimit = p.postLimit;
        error = p.error;
        description = p.description;
        referencedPosts = &p.referencedPosts;
        threadNumber = 0;
        postNumber = 0;
        if (board)
            processedText = Controller::processPostText(params.value("text"), board->name(), &refs);
    }
    explicit CreatePostInternalParameters(CreateThreadParameters &p, AbstractBoard *board) :
        request(p.request), params(p.params), files(p.files), locale(p.locale), fileTransaction(board)
    {
        error = p.error;
        description = p.description;
        bumpLimit = 0;
        postLimit = 0;
        threadNumber = 0;
        postNumber = 0;
        referencedPosts = 0;
        if (board)
            processedText = Controller::processPostText(params.value("text"), board->name(), &refs);
    }
};

static QReadWriteLock processTextLock(QReadWriteLock::Recursive);
static QMutex postMutex(QMutex::Recursive);
static QReadWriteLock rssLock(QReadWriteLock::Recursive);
static QMap<QString, QString> rssMap;

static bool addToReferencedPosts(QSharedPointer<Post> post, const RefMap &referencedPosts, QString *error = 0,
                                 QString *description = 0, const QLocale &l = BCoreApplication::locale())
{
    TranslatorQt tq(l);
    if (post.isNull()) {
        return bRet(error, tq.translate("addToReferencedPosts", "Internal error", "error"), description,
                    tq.translate("addToReferencedPosts", "Internal logic error", "description"), false);
    }
    if (referencedPosts.isEmpty())
        return bRet(error, QString(), description, QString(), true);
    try {
        Transaction t;
        if (!t) {
            return bRet(error, tq.translate("addToReferencedPosts", "Internal error", "error"), description,
                        tq.translate("addToReferencedPosts", "Internal database error", "description"), false);
        }
        foreach (const RefKey &key, referencedPosts.keys()) {
            if (!key.isValid()) {
                return bRet(error, tq.translate("addToReferencedPosts", "Internal error", "error"), description,
                            tq.translate("addToReferencedPosts", "Internal logic error", "description"), false);
            }
            Result<Post> p = queryOne<Post, Post>(odb::query<Post>::board == key.boardName
                                                     && odb::query<Post>::number == key.postNumber);
            if (p.error) {
                return bRet(error, tq.translate("addToReferencedPosts", "Internal error", "error"), description,
                            tq.translate("addToReferencedPosts", "Internal database error", "description"), false);
            }
            if (!p.data) {
                return bRet(error, tq.translate("addToReferencedPosts", "No such post", "error"), description,
                            tq.translate("addToReferencedPosts", "There is no such post", "description"), false);
            }
            Result<PostReference> ref = queryOne<PostReference, PostReference>(
                        odb::query<PostReference>::sourcePost == post->id()
                        && odb::query<PostReference>::targetPost == p.data->id());
            if (ref.error) {
                return bRet(error, tq.translate("addToReferencedPosts", "Internal error", "error"), description,
                            tq.translate("addToReferencedPosts", "Internal database error", "description"), false);
            }
            if (!ref.data) {
                QSharedPointer<PostReference> nref(new PostReference(post, p.data));
                t->persist(nref);
                Cache::removePost(key.boardName, key.postNumber);
            }
        }
        t.commit();
        return bRet(error, QString(), description, QString(), true);
    }  catch (const odb::exception &e) {
        return bRet(error, tq.translate("addToReferencedPosts", "Internal error", "error"), description,
                    Tools::fromStd(e.what()), false);
    }
}

static bool removeFromReferencedPosts(quint64 postId, QString *error = 0,
                                      const QLocale &l = BCoreApplication::locale())
{
    TranslatorQt tq(l);
    if (!postId)
        return bRet(error, tq.translate("removeFromReferencedPosts", "Internal logic error", "description"), false);
    try {
        Transaction t;
        if (!t) {
            return bRet(error, tq.translate("removeFromReferencedPosts", "Internal database error", "description"),
                        false);
        }
        QList<PostReference> posts = query<PostReference, PostReference>(
                    odb::query<PostReference>::sourcePost == postId);
        foreach (const PostReference &p, posts) {
            QSharedPointer<Post> sp = p.targetPost().load();
            Cache::removePost(sp->board(), sp->number());
        }
        t->erase_query<PostReference>(odb::query<PostReference>::sourcePost == postId);
        t.commit();
        return bRet(error, QString(), true);
    }  catch (const odb::exception &e) {
        return bRet(error, Tools::fromStd(e.what()), false);
    }
}

static bool copyFile(const QString &hashString, AbstractBoard::FileTransaction &ft, QString *error = 0,
                     QString *description = 0, const QLocale &l = BCoreApplication::locale())
{
    TranslatorQt tq(l);
    if (!ft.Board) {
        return bRet(error, tq.translate("copyFileHash", "Internal error", "error"), description,
                    tq.translate("copyFileHash", "Internal logic error", "description"), false);
    }
    bool ok = false;
    QByteArray fh = Tools::toHashpass(hashString, &ok);
    if (!ok || fh.isEmpty()) {
        return bRet(error, tq.translate("copyFileHash", "Invalid file hash", "error"), description,
                    tq.translate("copyFileHash", "Invalid file hash provided", "description"), false);
    }
    try {
        Transaction t;
        if (!t) {
            return bRet(error, tq.translate("copyFileHash", "Internal error", "error"), description,
                        tq.translate("copyFileHash", "Internal database error", "description"), false);
        }
        QList<FileInfo> fileInfos = query<FileInfo, FileInfo>(odb::query<FileInfo>::hash == fh);
        if (fileInfos.isEmpty()) {
            return bRet(error, tq.translate("copyFileHash", "No source file", "error"), description,
                        tq.translate("copyFileHash", "No source file for this file hash", "description"), false);
        }
        QString storagePath = Tools::storagePath();
        if (storagePath.isEmpty()) {
            return bRet(error, tq.translate("copyFileHash", "Internal error", "error"), description,
                        tq.translate("copyFileHash", "Internal file system error", "description"), false);
        }
        FileInfo info = fileInfos.first();
        QString sfn = storagePath + "/img/" + info.post().load()->board() + "/" + info.name();
        QFileInfo fi(sfn);
        QString path = fi.path();
        QString baseName = fi.baseName();
        QString suffix = fi.suffix();
        QStringList sl = BDirTools::entryList(path, QStringList() << (baseName + "s.*"), QDir::Files);
        if (!fi.exists() || sl.size() != 1) {
            return bRet(error, tq.translate("copyFileHash", "Internal error", "error"), description,
                        tq.translate("copyFileHash", "Internal file system error", "description"), false);
        }
        QString spfn = sl.first();
        QString dt = QString::number(QDateTime::currentDateTimeUtc().toMSecsSinceEpoch());
        path = QFileInfo(path).path() + "/" + ft.Board->name();
        QString fn = path + "/" + dt + "." + suffix;
        QString pfn = path + "/" + dt + "s." + QFileInfo(spfn).suffix();
        ft.addInfo(fn, fh, info.mimeType(), info.size());
        ft.setMainFileSize(info.height(), info.width());
        ft.setThumbFile(pfn);
        ft.setThumbFileSize(info.thumbHeight(), info.thumbWidth());
        ft.setMetaData(info.metaData());
        if (!BDirTools::mkpath(path) || !QFile::copy(sfn, fn) || !QFile::copy(spfn, pfn)) {
            return bRet(error, tq.translate("copyFileHash", "Internal error", "error"), description,
                        tq.translate("copyFileHash", "Internal file system error", "description"), false);
        }
        return bRet(error, QString(), description, QString(), true);
    } catch(const odb::exception &e) {
        return bRet(error, tq.translate("copyFile", "Internal error", "error"), description, Tools::fromStd(e.what()),
                    false);
    }
}

static QStringList deleteFileInfos(const Post &post, bool *ok = 0)
{
    typedef QLazyWeakPointer<FileInfo> FileInfoWP;
    QStringList list;
    try {
        Transaction t;
        if (!t)
            return bRet(ok, false, QStringList());
        foreach (FileInfoWP fi, post.fileInfos()) {
            QSharedPointer<FileInfo> fis = fi.load();
            list << fis->name() << fis->thumbName();
            t->erase(fis);
        }
        t.commit();
    } catch (const odb::exception &e) {
        Tools::log("Database::deleteFileInfos", e);
        return bRet(ok, false, QStringList());
    }
    return bRet(ok, true, list);
}

static void deleteFiles(const QString &boardName, const QStringList &fileNames)
{
    if (boardName.isEmpty())
        return;
    QString path = Tools::storagePath() + "/img/" + boardName;
    foreach (const QString &fn, fileNames)
        QFile::remove(path + "/" + fn);
}

static quint64 incrementPostCounter(const QString &boardName, quint64 delta = 1, QString *error = 0,
                                    const QLocale &l = BCoreApplication::locale())
{
    TranslatorQt tq(l);
    if (!AbstractBoard::boardNames().contains(boardName))
        return bRet(error, tq.translate("incrementPostCounter", "Invalid board name", "error"), 0L);
    if (!delta)
        return bRet(error, tq.translate("incrementPostCounter", "Internal logic error", "error"), 0L);
    quint64 incremented = 0L;
    try {
        Transaction t;
        if (!t)
            return bRet(error, tq.translate("incrementPostCounter", "Invalid database connection", "error"), 0L);
        Result<PostCounter> counter = queryOne<PostCounter, PostCounter>(odb::query<PostCounter>::board == boardName);
        if (!counter && !counter.error) {
            PostCounter c(boardName);
            t->persist(c);
            counter = queryOne<PostCounter, PostCounter>(odb::query<PostCounter>::board == boardName);
        }
        if (counter.error || !counter)
            return bRet(error, tq.translate("incrementPostCounter", "Internal database error", "error"), 0L);
        while (delta--)
            incremented = counter->incrementLastPostNumber();
        update(counter);
        t.commit();
    } catch (const odb::exception &e) {
        return bRet(error, Tools::fromStd(e.what()), 0L);
    }
    return bRet(error, QString(), incremented);
}

static bool saveFile(const Tools::File &f, AbstractBoard::FileTransaction &ft, QString *error = 0,
                     QString *description = 0, const QLocale &l = BCoreApplication::locale())
{
    TranslatorQt tq(l);
    if (!ft.Board) {
        return bRet(error, tq.translate("saveFile", "Internal error", "error"), description,
                    tq.translate("saveFile", "Internal logic error", "description"), false);
    }
    if (!ft.Board->saveFile(f, ft)) {
        return bRet(error, tq.translate("saveFile", "Internal error", "error"), description,
                    tq.translate("saveFile", "Internal file system error", "description"), false);
    }
    return bRet(error, QString(), description, QString(), true);
}

static bool saveFiles(const QMap<QString, QString> &params, const Tools::FileList &files,
                      AbstractBoard::FileTransaction &ft, QString *error = 0, QString *description = 0,
                      const QLocale &l = BCoreApplication::locale())
{
    Tools::Post post = Tools::toPost(params, files);
    foreach (const Tools::File &f, post.files) {
        if (!saveFile(f, ft, error, description, l))
            return false;
    }
    foreach (const QString &fhs, post.fileHashes) {
        if (!copyFile(fhs, ft, error, description, l))
            return false;
    }
    return bRet(error, QString(), description, QString(), true);
}

static bool testCaptcha(const cppcms::http::request &req, const Tools::PostParameters &params, QString *error = 0,
                        QString *description = 0, const QLocale &l = BCoreApplication::locale())
{
    TranslatorQt tq(l);
    AbstractBoard::LockingWrapper board = AbstractBoard::board(params.value("board"));
    if (board.isNull()) {
        return bRet(error, tq.translate("testCaptcha", "Internal error", "error"), description,
                    tq.translate("testCaptcha", "Internal logic error", "description"), false);
    }
    if (!Tools::captchaEnabled(board->name()))
        return bRet(error, QString(), description, QString(), true);
    QString ip = Tools::userIp(req);
    if (board->captchaQuota(ip)) {
        board->captchaUsed(ip);
    } else {
        QStringList supportedCaptchaEngines = board->supportedCaptchaEngines().split(',');
        if (supportedCaptchaEngines.isEmpty()) {
            return bRet(error, tq.translate("testCaptcha", "Internal error", "error"), description,
                        tq.translate("testCaptcha", "Internal logic error", "description"), false);
        }
        QString ceid = params.value("captchaEngine");
        if (ceid.isEmpty() || !supportedCaptchaEngines.contains(ceid, Qt::CaseInsensitive)) {
            if (supportedCaptchaEngines.contains("google-recaptcha"))
                ceid = "google-recaptcha";
            else
                ceid = supportedCaptchaEngines.first();
        }
        AbstractCaptchaEngine::LockingWrapper e = AbstractCaptchaEngine::engine(ceid);
        if (e.isNull()) {
            return bRet(error, tq.translate("testCaptcha", "Invalid captcha", "error"), description,
                        tq.translate("testCaptcha", "No engine for this captcha type", "sescription"), false);
        }
        QString err;
        if (!e->checkCaptcha(req, params, err))
            return bRet(error, tq.translate("testCaptcha", "Invalid captcha", "error"), description, err, false);
        board->captchaSolved(ip);
    }
    return bRet(error, QString(), description, QString(), true);
}

static bool banUserInternal(const QString &sourceBoard, quint64 postNumber, const QString &board, int level,
                            const QString &reason, const QDateTime &expires, QString *error, const QLocale &l,
                            QString ip = QString())
{
    TranslatorQt tq(l);
    if (board.isEmpty() || (!AbstractBoard::boardNames().contains(board) && "*" != board))
        return bRet(error, tq.translate("banUserInternal", "Invalid board name", "error"), false);
    try {
        Transaction t;
        if (!t)
            return bRet(error, tq.translate("banUserInternal", "Internal database error", "error"), false);
        quint64 postId = 0L;
        if (!sourceBoard.isEmpty() && postNumber) {
            Result<Post> post = queryOne<Post, Post>(odb::query<Post>::board == sourceBoard
                                                     && odb::query<Post>::number == postNumber);
            if (post.error)
                return bRet(error, tq.translate("banUserInternal", "Internal database error", "error"), false);
            if (!post.data)
                return bRet(error, tq.translate("banUserInternal", "No such post", "error"), false);
            postId = post->id();
            if (ip.isEmpty())
                ip = post->posterIp();
            post->setBannedFor(level > 0);
            update(post);
        }
        if (ip.isEmpty())
            return bRet(error, tq.translate("banUserInternal", "Invalid IP address", "error"), false);
        Result<BannedUser> user = queryOne<BannedUser, BannedUser>(odb::query<BannedUser>::board == board
                                                                   && odb::query<BannedUser>::ip == ip);
        if (user.error)
            return bRet(error, tq.translate("banUserInternal", "Internal database error", "error"), false);
        QDateTime dt = QDateTime::currentDateTimeUtc();
        if (!user) {
            if (level < 1 || (expires.isValid() && expires.toUTC() <= dt)) {
                t.commit();
                return bRet(error, QString(), true);
            }
            user = new BannedUser(board, ip, dt, postId);
            user->setExpirationDateTime(expires);
            user->setLevel(level);
            user->setReason(reason);
            persist(user);
        } else {
            if (level < 1 || (expires.isValid() && expires.toUTC() <= dt)) {
                t->erase(*user);
                t.commit();
                Cache::removePost(board, postNumber);
                return bRet(error, QString(), true);
            }
            user->setDateTime(dt);
            user->setExpirationDateTime(expires);
            user->setLevel(level);
            user->setReason(reason);
            update(user);
        }
        t.commit();
        Cache::removePost(board, postNumber);
        return bRet(error, QString(), true);
    }  catch (const odb::exception &e) {
        return bRet(error, Tools::fromStd(e.what()), false);
    }
}

static bool createPostInternal(CreatePostInternalParameters &p)
{
    QString boardName = p.params.value("board");
    AbstractBoard::LockingWrapper board = AbstractBoard::board(boardName);
    TranslatorQt tq(p.locale);
    bSet(p.postNumber, quint64(0L));
    if (board.isNull()) {
        return bRet(p.error, tq.translate("createPostInternal", "Internal error", "error"), p.description,
                           tq.translate("createPostInternal", "Internal logic error", "description"), false);
    }
    bool isThread = p.threadNumber;
    if (!p.threadNumber)
        p.threadNumber = p.params.value("thread").toULongLong();
    Tools::Post post = Tools::toPost(p.params, p.files);
    bool draft = board->draftsEnabled() && post.draft;
    RefMap refs;
    QString processedText = p.processedText;
    if (!draft)
        refs = p.refs;
    QReadLocker locker(&processTextLock);
    try {
        Transaction t;
        if (!t) {
            return bRet(p.error, tq.translate("createPostInternal", "Internal error", "error"), p.description,
                        tq.translate("createPostInternal", "Internal database error", "description"), false);
        }
        QString err;
        quint64 postNumber = p.dateTime.isValid() ? lastPostNumber(boardName, &err, tq.locale())
                                                  : incrementPostCounter(boardName, 1, &err, tq.locale());
        if (!postNumber) {
            return bRet(p.error, tq.translate("createPostInternal", "Internal error", "error"), p.description, err,
                        false);
        }
        Result<Thread> thread = queryOne<Thread, Thread>(odb::query<Thread>::board == boardName
                                                         && odb::query<Thread>::number == p.threadNumber);
        if (thread.error) {
            return bRet(p.error, tq.translate("createPostInternal", "Internal error", "error"), p.description,
                        tq.translate("createPostInternal", "Internal database error", "description"), false);
        }
        if (!thread) {
            return bRet(p.error, tq.translate("createPostInternal", "No such thread", "error"), p.description,
                        tq.translate("createPostInternal", "There is no such thread", "description"), false);
        }
        bool bump = post.email.compare("sage", Qt::CaseInsensitive);
        if (p.postLimit || p.bumpLimit) {
            Result<PostCount> postCount = queryOne<PostCount, Post>(odb::query<Post>::thread == thread->id());
            if (postCount.error || !postCount) {
                return bRet(p.error, tq.translate("createPostInternal", "Internal error", "error"), p.description,
                            tq.translate("createPostInternal", "Internal database error", "description"), false);
            }
            if (p.postLimit && (postCount->count >= (int) p.postLimit)) {
                return bRet(p.error, tq.translate("createPostInternal", "Post limit", "error"), p.description,
                            tq.translate("createPostInternal", "The thread has reached it's post limit",
                                         "description"), false);
            }
            if (p.bumpLimit && (postCount->count >= (int) p.bumpLimit))
                bump = false;
        }
        if (!p.dateTime.isValid())
            p.dateTime = QDateTime::currentDateTimeUtc();
        QByteArray hp = Tools::hashpass(p.request);
        QString ip = Tools::userIp(p.request);
        GeolocationInfo gli = geolocationInfo(ip);
        QSharedPointer<Post> ps(new Post(boardName, postNumber, p.dateTime, thread.data, ip, gli.countryCode,
                                         gli.countryName, gli.cityName, post.password, hp));
        ps->setEmail(post.email);
        ps->setName(post.name);
        ps->setSubject(post.subject);
        ps->setRawText(post.text);
        if (draft)
            ps->setDraft(true);
        bool raw = post.raw && registeredUserLevel(p.request) >= RegisteredUser::AdminLevel;
        if (raw) {
            ps->setText(post.text);
            ps->setRawHtml(true);
        } else {
            ps->setText(processedText);
            bSet(p.referencedPosts, refs);
        }
        ps->setShowTripcode(post.showTripcode);
        if (bump) {
            thread->setDateTime(p.dateTime);
            update(thread);
        }
        if (!board->beforeStoringNewPost(p.request, ps.data(), p.params, isThread, p.error, p.description))
            return false;
        t->persist(ps);
        foreach (const AbstractBoard::FileInfo &fi, p.fileTransaction.fileInfos()) {
            FileInfo fileInfo(fi.name, fi.hash, fi.mimeType, fi.size, fi.height, fi.width, fi.thumbName,
                              fi.thumbHeight, fi.thumbWidth, fi.metaData, fi.rating, ps);
            t->persist(fileInfo);
        }
        if (!raw && !ps->draft() && !addToReferencedPosts(ps, refs, p.error, p.description, tq.locale()))
            return false;
        bSet(p.postNumber, postNumber);
        t.commit();
        p.fileTransaction.commit();
        Search::addToIndex(boardName, postNumber, post.text);
        return bRet(p.error, QString(), p.description, QString(), true);
    } catch (const odb::exception &e) {
        return bRet(p.error, tq.translate("createPostInternal", "Internal error", "error"), p.description,
                    Tools::fromStd(e.what()), false);
    }
}

static bool deletePostInternal(const QString &boardName, quint64 postNumber, QString *error, const QLocale &l,
                               QStringList &filesToDelete)
{
    TranslatorQt tq(l);
    if (boardName.isEmpty() || !AbstractBoard::boardNames().contains(boardName))
        return bRet(error, tq.translate("deletePostInternal", "Invalid board name", "error"), false);
    AbstractBoard::LockingWrapper board = AbstractBoard::board(boardName);
    if (board.isNull())
        return bRet(error, tq.translate("deletePostInternal", "Internal logic error", "error"), false);
    if (!postNumber)
        return bRet(error, tq.translate("deletePostInternal", "Invalid post number", "error"), false);
    QWriteLocker lock(&processTextLock);
    QMap<quint64, PostTmpInfo> postIds;
    QMutexLocker locker(&postMutex);
    try {
        Transaction t;
        if (!t)
            return bRet(error, tq.translate("deletePostInternal", "Internal database error", "error"), false);
        Result<Post> post = queryOne<Post, Post>(odb::query<Post>::board == boardName
                                                 && odb::query<Post>::number == postNumber);
        if (post.error)
            return bRet(error, tq.translate("deletePostInternal", "Internal database error", "error"), false);
        if (!post)
            return bRet(error, tq.translate("deletePostInternal", "No such post", "error"), false);
        Result<Thread> thread = queryOne<Thread, Thread>(odb::query<Thread>::board == boardName
                                                         && odb::query<Thread>::number == postNumber);
        if (thread.error)
            return bRet(error, tq.translate("deletePostInternal", "Internal database error", "error"), false);
        if (!removeFromReferencedPosts(post.data->id(), error, tq.locale()))
            return false;
        typedef QLazySharedPointer<PostReference> PostReferenceSP;
        foreach (PostReferenceSP ref, post->referencedBy()) {
            PostTmpInfo tmp;
            Post p = *ref.load()->sourcePost().load();
            tmp.board = p.board();
            tmp.text = p.rawText();
            postIds.insert(p.id(), tmp);
        }
        t.commit();
    }  catch (const odb::exception &e) {
        return bRet(error, Tools::fromStd(e.what()), false);
    }
    foreach (quint64 id, postIds.keys()) {
        PostTmpInfo &tmp = postIds[id];
        tmp.text = Controller::processPostText(tmp.text, tmp.board, 0, postNumber);
    }
    try {
        Transaction t;
        if (!t)
            return bRet(error, tq.translate("deletePostInternal", "Internal database error", "error"), false);
        Result<Post> post = queryOne<Post, Post>(odb::query<Post>::board == boardName
                                                 && odb::query<Post>::number == postNumber);
        if (post.error)
            return bRet(error, tq.translate("deletePostInternal", "Internal database error", "error"), false);
        if (!post)
            return bRet(error, tq.translate("deletePostInternal", "No such post", "error"), false);
        Result<Thread> thread = queryOne<Thread, Thread>(odb::query<Thread>::board == boardName
                                                         && odb::query<Thread>::number == postNumber);
        if (thread.error)
            return bRet(error, tq.translate("deletePostInternal", "Internal database error", "error"), false);
        typedef QLazySharedPointer<PostReference> PostReferenceSP;
        QList<Post> referenced;
        foreach (PostReferenceSP ref, post->referencedBy())
            referenced << *ref.load()->sourcePost().load();
        t->erase_query<PostReference>(odb::query<PostReference>::targetPost == post.data->id());
        if (thread) {
            QList<Post> posts = query<Post, Post>(odb::query<Post>::thread == thread->id());
            foreach (const Post &p, posts) {
                bool ok = false;
                filesToDelete << deleteFileInfos(p, &ok);
                if (!ok)
                    return bRet(error, tq.translate("deletePostInternal", "Internal database error", "error"), false);
                t->erase_query<PostReference>(odb::query<PostReference>::sourcePost == p.id());
                t->erase_query<PostReference>(odb::query<PostReference>::targetPost == p.id());
                Cache::removePost(p.board(), p.number());
            }
            t->erase_query<Post>(odb::query<Post>::thread == thread->id());
        } else {
            bool ok = false;
            filesToDelete << deleteFileInfos(*post, &ok);
            if (!ok)
                return bRet(error, tq.translate("deletePostInternal", "Internal database error", "error"), false);
            t->erase_query<Post>(odb::query<Post>::board == boardName && odb::query<Post>::number == postNumber);
        }
        foreach (Post p, referenced) {
            if (thread && p.thread().load()->id() == thread->id())
                continue;
            if (p.rawHtml())
                continue;
            p.setText(postIds.value(p.id()).text);
            Cache::removePost(p.board(), p.number());
            t->update(p);
        }
        if (thread)
            t->erase_query<Thread>(odb::query<Thread>::id == thread->id());
        Cache::removePost(boardName, postNumber);
        Search::removeFromIndex(boardName, postNumber, post->text());
        t.commit();
        return bRet(error, QString(), true);
    }  catch (const odb::exception &e) {
        return bRet(error, Tools::fromStd(e.what()), false);
    }
}

static bool setThreadFixedInternal(const QString &board, quint64 threadNumber, bool fixed, QString *error,
                                   const QLocale &l)
{
    TranslatorQt tq(l);
    try {
        Transaction t;
        if (!t)
            return bRet(error, tq.translate("setThreadFixedInternal", "Internal database error", "error"), false);
        Result<Thread> thread = queryOne<Thread, Thread>(odb::query<Thread>::board == board
                                                         && odb::query<Thread>::number == threadNumber);
        if (thread.error)
            return bRet(error, tq.translate("setThreadFixedInternal", "Internal database error", "error"), false);
        if (!thread)
            return bRet(error, tq.translate("setThreadFixedInternal", "No such thread", "error"), false);
        if (thread->fixed() == fixed)
            return bRet(error, QString(), true);
        thread->setFixed(fixed);
        update(thread);
        t.commit();
        Cache::removePost(board, threadNumber);
        return bRet(error, QString(), true);
    } catch (const odb::exception &e) {
        return bRet(error, Tools::fromStd(e.what()), false);
    }
}

static bool setThreadOpenedInternal(const QString &board, quint64 threadNumber, bool opened, QString *error,
                                    const QLocale &l)
{
    TranslatorQt tq(l);
    try {
        Transaction t;
        if (!t)
            return bRet(error, tq.translate("setThreadOpenedInternal", "Internal database error", "error"), false);
        Result<Thread> thread = queryOne<Thread, Thread>(odb::query<Thread>::board == board
                                                         && odb::query<Thread>::number == threadNumber);
        if (thread.error)
            return bRet(error, tq.translate("setThreadOpenedInternal", "Internal database error", "error"), false);
        if (!thread)
            return bRet(error, tq.translate("setThreadOpenedInternal", "No such thread", "error"), false);
        if (thread->postingEnabled() == opened)
            return bRet(error, QString(), true);
        thread->setPostingEnabled(opened);
        update(thread);
        t.commit();
        Cache::removePost(board, threadNumber);
        return bRet(error, QString(), true);
    } catch (const odb::exception &e) {
        return bRet(error, Tools::fromStd(e.what()), false);
    }
}

static bool threadIdDateTimeFixedLessThan(const ThreadIdDateTimeFixed &t1, const ThreadIdDateTimeFixed &t2)
{
    if (t1.fixed == t2.fixed)
        return t1.dateTime.toUTC() > t2.dateTime.toUTC();
    else if (t1.fixed)
        return true;
    else
        return false;
}

bool addFile(const cppcms::http::request &req, const QMap<QString, QString> &params, const QList<Tools::File> &files,
             QString *error, QString *description)
{
    TranslatorQt tq(req);
    AbstractBoard::LockingWrapper board = AbstractBoard::board(params.value("board"));
    if (board.isNull()) {
        return bRet(error, tq.translate("addFile", "Internal error", "error"), description,
                    tq.translate("addFile", "Internal logic error", "description"), false);
    }
    AbstractBoard::FileTransaction ft(board.data());
    if (!saveFiles(params, files, ft, error, description, tq.locale()))
        return false;
    try {
        Transaction t;
        quint64 postNumber = params.value("postNumber").toULongLong();
        if (!postNumber) {
            return bRet(error, tq.translate("addFile", "Invalid parameters", "error"), description,
                        tq.translate("addFile", "Invalid post number", "description"), false);
        }
        Result<Post> post = queryOne<Post, Post>(odb::query<Post>::board == board->name()
                                                 && odb::query<Post>::number == postNumber);
        if (post.error) {
            return bRet(error, tq.translate("addFile", "Internal error", "error"), description,
                        tq.translate("addFile", "Internal database error", "error"), false);
        }
        if (!post) {
            return bRet(error, tq.translate("addFile", "Invalid parameters", "error"), description,
                        tq.translate("addFile", "No such post", "error"), false);
        }
        QList<AbstractBoard::FileInfo> infos = ft.fileInfos();
        if ((infos.size() + post->fileInfos().size()) > int(Tools::maxInfo(Tools::MaxFileCount, board->name()))) {
            return bRet(error, tq.translate("addFile", "Invalid parameters", "error"), description,
                        tq.translate("addFile", "Too many files", "error"), false);
        }
        foreach (const AbstractBoard::FileInfo &fi, infos) {
            FileInfo fileInfo(fi.name, fi.hash, fi.mimeType, fi.size, fi.height, fi.width, fi.thumbName,
                              fi.thumbHeight, fi.thumbWidth, fi.metaData, fi.rating, post.data);
            t->persist(fileInfo);
        }
        t.commit();
        ft.commit();
        Cache::removePost(post->board(), post->number());
        return bRet(error, QString(), description, QString(), true);
    } catch (const odb::exception &e) {
        return bRet(error, tq.translate("addFile", "Internal error", "error"), description, Tools::fromStd(e.what()),
                    false);
    }
}

int addPostsToIndex(QString *error, const QLocale &l)
{
    TranslatorQt tq(l);
    try {
        Transaction t;
        if (!t)
            return bRet(error, tq.translate("addPostsToIndex", "Internal database error", "error"), -1);
        QList<Post> posts = queryAll<Post>();
        foreach (const Post &post, posts)
            Search::addToIndex(post.board(), post.number(), post.rawText());
        return bRet(error, QString(), posts.size());
    } catch (const odb::exception &e) {
        return bRet(error, Tools::fromStd(e.what()), -1);
    }
}

bool banUser(const QString &ip, const QString &board, int level, const QString &reason, const QDateTime &expires,
             QString *error, const QLocale &l)
{
    return banUserInternal("", 0, board, level, reason, expires, error, l, ip);
}

bool banUser(const QString &sourceBoard, quint64 postNumber, const QString &board, int level, const QString &reason,
             const QDateTime &expires, QString *error, const QLocale &l)
{
    return banUserInternal(sourceBoard, postNumber, board, level, reason, expires, error, l);
}

bool banUser(const cppcms::http::request &req, const QString &sourceBoard, quint64 postNumber, const QString &board,
             int level, const QString &reason, const QDateTime &expires, QString *error)
{
    TranslatorQt tq(req);
    QStringList boardNames = AbstractBoard::boardNames();
    if (board != "*" && (!boardNames.contains(board) || !boardNames.contains(sourceBoard)))
        return bRet(error, tq.translate("banUser", "Invalid board name", "error"), false);
    if (!postNumber)
        return bRet(error, tq.translate("banUser", "Invalid post number", "error"), false);
    QByteArray hashpass = Tools::hashpass(req);
    if (hashpass.isEmpty())
        return bRet(error, tq.translate("banUser", "Not logged in", "error"), false);
    try {
        Transaction t;
        if (!t)
            return bRet(error, tq.translate("banUser", "Internal database error", "error"), false);
        Result<Post> post = queryOne<Post, Post>(odb::query<Post>::board == sourceBoard
                                                 && odb::query<Post>::number == postNumber);
        if (post.error)
            return bRet(error, tq.translate("banUser", "Internal database error", "error"), false);
        if (!post)
            return bRet(error, tq.translate("banUser", "No such post", "error"), false);
        if (hashpass == post->hashpass())
            return bRet(error, tq.translate("banUser", "You can't ban youself, baka", "error"), false);
        int lvl = registeredUserLevel(req);
        if (!moderOnBoard(req, board, sourceBoard) || registeredUserLevel(post->hashpass()) >= lvl)
            return bRet(error, tq.translate("banUser", "Not enough rights", "error"), false);
        if (!banUser(sourceBoard, postNumber, board, level, reason, expires, error, tq.locale()))
            return false;
        t.commit();
        return bRet(error, QString(), true);
    } catch (const odb::exception &e) {
        return bRet(error, Tools::fromStd(e.what()), false);
    }
}

void checkOutdatedEntries()
{
    try {
        Transaction t;
        if (!t)
            return;
        QDateTime dt = QDateTime::currentDateTimeUtc();
        QList<BannedUser> list = queryAll<BannedUser>();
        foreach (const BannedUser &u, list) {
            QDateTime exp = u.expirationDateTime();
            if (exp.isValid() && exp <= dt) {
                quint64 postId = u.postId();
                t.db()->erase_query<BannedUser>(odb::query<BannedUser>::id == u.id());
                if (postId) {
                    Result<Post> post = queryOne<Post, Post>(odb::query<Post>::id == postId);
                    if (post.error || !post)
                        continue;
                    post->setBannedFor(false);
                    update(post);
                    Cache::removePost(post->board(), post->number());
                }
            }
        }
        t.commit();
    } catch (const odb::exception &e) {
        Tools::log("Database::checkOutdatedEntries", e);
    }
}

bool createPost(CreatePostParameters &p, quint64 *postNumber)
{
    bSet(postNumber, quint64(0L));
    if (!testCaptcha(p.request, p.params, p.error, p.description, p.locale))
        return false;
    TranslatorQt tq(p.locale);
    AbstractBoard::LockingWrapper board = AbstractBoard::board(p.params.value("board"));
    if (board.isNull()) {
        return bRet(p.error, tq.translate("createPost", "Internal error", "error"), p.description,
                    tq.translate("createPost", "Internal logic error", "description"), false);
    }
    CreatePostInternalParameters pp(p, board.data());
    if (!saveFiles(p.params, p.files, pp.fileTransaction, p.error, p.description, p.locale))
        return false;
    pp.postNumber = postNumber;
    QMutexLocker locker(&postMutex);
    if (!createPostInternal(pp))
        return false;
    return bRet(p.error, QString(), p.description, QString(), true);
}

void createSchema()
{
    try {
        Transaction t;
        if (!t)
            return;
        if (t->execute("SELECT 1 FROM sqlite_master WHERE type='table' AND name='threads'"))
            return;
        t->execute("PRAGMA foreign_keys=OFF");
        odb::schema_catalog::create_schema(*t);
        t->execute("PRAGMA foreign_keys=ON");
        t.commit();
    } catch (const odb::exception &e) {
        Tools::log("Database::createSchema", e);
    }
}

quint64 createThread(CreateThreadParameters &p)
{
    if (!testCaptcha(p.request, p.params, p.error, p.description, p.locale))
        return false;
    QString boardName = p.params.value("board");
    AbstractBoard::LockingWrapper board = AbstractBoard::board(boardName);
    TranslatorQt tq(p.locale);
    if (board.isNull()) {
        return bRet(p.error, tq.translate("createThread", "Internal error", "error"), p.description,
                    tq.translate("createThread", "Internal logic error", "description"), 0L);
    }
    CreatePostInternalParameters pp(p, board.data());
    if (!saveFiles(p.params, p.files, pp.fileTransaction, p.error, p.description, p.locale))
        return false;
    QMutexLocker locker(&postMutex);
    try {
        QStringList filesToDelete;
        Transaction t;
        if (!t)
            return bRet(p.error, tq.translate("createThread", "Internal database error", "error"), p.description,
                        tq.translate("createThread", "Internal database error", "error"), false);
        QString err;
        quint64 postNumber = incrementPostCounter(p.params.value("board"), 1, &err, p.locale);
        if (!postNumber)
            return bRet(p.error, tq.translate("createThread", "Internal error", "error"), p.description, err, 0L);
        if (p.threadLimit) {
            Result<ThreadCount> threadCount = queryOne<ThreadCount, Thread>(odb::query<Thread>::board == boardName
                                                                            && odb::query<Thread>::archived == false);
            if (threadCount.error || !threadCount) {
                return bRet(p.error, tq.translate("createThread", "Internal error", "error"),
                            p.description, tq.translate("createThread", "Internal database error", "error"), 0L);
            }
            if (threadCount->count >= (int) p.threadLimit) {
                QList<ThreadIdDateTimeFixed> list = query<ThreadIdDateTimeFixed, Thread>(
                            odb::query<Thread>::board == boardName && odb::query<Thread>::archived == false);
                qSort(list.begin(), list.end(), &threadIdDateTimeFixedLessThan);
                if (p.archiveLimit) {
                    Result<ThreadCount> archivedCount = queryOne<ThreadCount, Thread>(
                                odb::query<Thread>::board == boardName && odb::query<Thread>::archived == true);
                    if (archivedCount.error || !archivedCount) {
                        return bRet(p.error, tq.translate("createThread", "Internal error", "error"), p.description,
                                    tq.translate("createThread", "Internal database error", "error"), 0L);
                    }
                    if (archivedCount->count >= (int) p.archiveLimit) {
                        QList<ThreadIdDateTimeFixed> archivedList = query<ThreadIdDateTimeFixed, Thread>(
                                    odb::query<Thread>::board == boardName && odb::query<Thread>::archived == true);
                        qSort(archivedList.begin(), archivedList.end(), &threadIdDateTimeFixedLessThan);
                        if (!deletePostInternal(boardName, archivedList.last().number, p.description, p.locale,
                                                filesToDelete)) {
                            return bRet(p.error, tq.translate("createThread", "Internal error", "error"), 0L);
                        }
                    }
                    Result<Thread> thread = queryOne<Thread, Thread>(odb::query<Thread>::id == list.last().id);
                    if (thread.error) {
                        return bRet(p.error, tq.translate("createThread", "Internal error", "error"), p.description,
                                    tq.translate("createThread", "Internal database error", "error"), 0L);
                    }
                    if (!thread) {
                        return bRet(p.error, tq.translate("createThread", "Internal error", "error"), p.description,
                                    tq.translate("createThread", "Internal database error", "error"), 0L);
                    }
                    thread->setArchived(true);
                    update(thread);
                } else {
                    if (!deletePostInternal(boardName, list.last().number, p.description, p.locale, filesToDelete))
                        return bRet(p.error, tq.translate("createThread", "Internal error", "error"), 0L);
                }
            }
        }
        QDateTime dt = QDateTime::currentDateTimeUtc();
        QSharedPointer<Thread> thread(new Thread(p.params.value("board"), postNumber, dt));
        if (board->draftsEnabled() && p.params.value("draft").compare("true", Qt::CaseInsensitive))
            thread->setDraft(true);
        t->persist(thread);
        pp.dateTime = dt;
        pp.threadNumber = postNumber;
        if (!createPostInternal(pp))
            return 0L;
        t.commit();
        deleteFiles(boardName, filesToDelete);
        return bRet(p.error, QString(), p.description, QString(), postNumber);
    } catch (const odb::exception &e) {
        return bRet(p.error, tq.translate("createThread", "Internal error", "error"), p.description,
                    Tools::fromStd(e.what()), 0L);
    }
}

bool deleteFile(const QString &boardName, const QString &fileName,  const cppcms::http::request &req,
                const QByteArray &password, QString *error)
{
    TranslatorQt tq(req);
    if (!AbstractBoard::boardNames().contains(boardName))
        return bRet(error, tq.translate("deleteFile", "Invalid board name", "error"), false);
    if (fileName.isEmpty())
        return bRet(error, tq.translate("deleteFile", "Invalid file name", "error"), false);
    QByteArray hashpass = Tools::hashpass(req);
    if (password.isEmpty() && hashpass.isEmpty())
        return bRet(error, tq.translate("deleteFile", "Invalid password", "error"), false);
    try {
        Transaction t;
        if (!t)
            return bRet(error, tq.translate("deleteFile", "Internal database error", "error"), false);
        Result<FileInfo> fileInfo = queryOne<FileInfo, FileInfo>(odb::query<FileInfo>::name == fileName);
        if (fileInfo.error)
            return bRet(error, tq.translate("deleteFile", "Internal database error", "error"), false);
        if (!fileInfo)
            return bRet(error, tq.translate("deleteFile", "No such file", "error"), false);
        QSharedPointer<Post> post = fileInfo->post().load();
        if (post->board() != boardName)
            return bRet(error, tq.translate("deleteFile", "Board name mismatch", "error"), false);
        if (password.isEmpty()) {
            if (hashpass != post->hashpass()) {
                int lvl = registeredUserLevel(req);
                if (!moderOnBoard(req, boardName) || registeredUserLevel(post->hashpass()) >= lvl)
                    return bRet(error, tq.translate("deleteFile", "Not enough rights", "error"), false);
            }
        } else if (password != post->password()) {
            return bRet(error, tq.translate("deleteFile", "Incorrect password", "error"), false);
        }
        erase(fileInfo);
        t.commit();
        Cache::removePost(boardName, post->number());
        deleteFiles(boardName, QStringList() << fileInfo->name() << fileInfo->thumbName());
        return bRet(error, QString(), true);
    } catch (const odb::exception &e) {
        return bRet(error, Tools::fromStd(e.what()), false);
    }
}

bool deletePost(const QString &boardName, quint64 postNumber, QString *error, const QLocale &l)
{
    QStringList list;
    bool b = deletePostInternal(boardName, postNumber, error, l, list);
    deleteFiles(boardName, list);
    return b;
}

bool deletePost(const QString &boardName, quint64 postNumber, const cppcms::http::request &req,
                const QByteArray &password, QString *error)
{
    TranslatorQt tq(req);
    if (!AbstractBoard::boardNames().contains(boardName))
        return bRet(error, tq.translate("deletePost", "Invalid board name", "error"), false);
    if (!postNumber)
        return bRet(error, tq.translate("deletePost", "Invalid post number", "error"), false);
    QByteArray hashpass = Tools::hashpass(req);
    if (password.isEmpty() && hashpass.isEmpty())
        return bRet(error, tq.translate("deletePost", "Invalid password", "error"), false);
    QStringList filesToDelete;
    QMutexLocker locker(&postMutex);
    try {
        Transaction t;
        if (!t)
            return bRet(error, tq.translate("deletePost", "Internal database error", "error"), false);
        Result<Post> post = queryOne<Post, Post>(odb::query<Post>::board == boardName
                                                 && odb::query<Post>::number == postNumber);
        if (post.error)
            return bRet(error, tq.translate("deletePost", "Internal database error", "error"), false);
        if (!post)
            return bRet(error, tq.translate("deletePost", "No such post", "error"), false);
        if (password.isEmpty()) {
            if (hashpass != post->hashpass()) {
                int lvl = registeredUserLevel(req);
                if (!moderOnBoard(req, boardName) || registeredUserLevel(post->hashpass()) >= lvl)
                    return bRet(error, tq.translate("deletePost", "Not enough rights", "error"), false);
            }
        } else if (password != post->password()) {
            return bRet(error, tq.translate("deletePost", "Incorrect password", "error"), false);
        }
    } catch (const odb::exception &e) {
        return bRet(error, Tools::fromStd(e.what()), false);
    }
    if (!deletePostInternal(boardName, postNumber, error, tq.locale(), filesToDelete))
        return false;
    deleteFiles(boardName, filesToDelete);
    return bRet(error, QString(), true);
}

bool editAudioTags(const QString &boardName, const QString &fileName, const cppcms::http::request &req,
                   const QByteArray &password, const QVariantMap &tags, QString *error)
{
    TranslatorQt tq(req);
    if (!AbstractBoard::boardNames().contains(boardName))
        return bRet(error, tq.translate("editAudioTags", "Invalid board name", "error"), false);
    if (fileName.isEmpty())
        return bRet(error, tq.translate("editAudioTags", "Invalid file name", "error"), false);
    QByteArray hashpass = Tools::hashpass(req);
    if (password.isEmpty() && hashpass.isEmpty())
        return bRet(error, tq.translate("editAudioTags", "Invalid password", "error"), false);
    try {
        Transaction t;
        if (!t)
            return bRet(error, tq.translate("editAudioTags", "Internal database error", "error"), false);
        Result<FileInfo> fileInfo = queryOne<FileInfo, FileInfo>(odb::query<FileInfo>::name == fileName);
        if (fileInfo.error)
            return bRet(error, tq.translate("editAudioTags", "Internal database error", "error"), false);
        if (!fileInfo)
            return bRet(error, tq.translate("editAudioTags", "No such file", "error"), false);
        if (!fileInfo->mimeType().startsWith("audio/"))
            return bRet(error, tq.translate("editAudioTags", "Not an audio file", "error"), false);
        QSharedPointer<Post> post = fileInfo->post().load();
        if (post->board() != boardName)
            return bRet(error, tq.translate("editAudioTags", "Board name mismatch", "error"), false);
        if (password.isEmpty()) {
            if (hashpass != post->hashpass()) {
                int lvl = registeredUserLevel(req);
                if (!moderOnBoard(req, boardName) || registeredUserLevel(post->hashpass()) >= lvl)
                    return bRet(error, tq.translate("editAudioTags", "Not enough rights", "error"), false);
            }
        } else if (password != post->password()) {
            return bRet(error, tq.translate("editAudioTags", "Incorrect password", "error"), false);
        }
        QVariantMap m = fileInfo->metaData().toMap();
        static const QStringList Keys = QStringList() << "album" << "artist" << "title" << "year";
        foreach (const QString &key, Keys) {
            if (!tags.contains(key))
                continue;
            QVariant v = tags.value(key);
            if (v.type() != QVariant::String)
                continue;
            m[key] = v;
        }
        fileInfo->setMetaData(m);
        update(fileInfo);
        t.commit();
        Cache::removePost(boardName, post->number());
        return bRet(error, QString(), true);
    } catch (const odb::exception &e) {
        return bRet(error, Tools::fromStd(e.what()), false);
    }
}

bool editPost(EditPostParameters &p)
{
    AbstractBoard::LockingWrapper board = AbstractBoard::board(p.boardName);
    TranslatorQt tq(p.request);
    if (board.isNull())
        return bRet(p.error, tq.translate("editPost", "Invalid board name", "error"), false);
    if (!p.postNumber)
        return bRet(p.error, tq.translate("editPost", "Invalid post number", "error"), false);
    QByteArray hashpass = Tools::hashpass(p.request);
    if (p.password.isEmpty() && hashpass.isEmpty())
        return bRet(p.error, tq.translate("editPost", "Invalid password", "error"), false);
    QString processedText = Controller::processPostText(p.text, p.boardName, &p.referencedPosts);
    QReadLocker locker(&processTextLock);
    QMutexLocker plocker(&postMutex);
    try {
        Transaction t;
        if (!t)
            return bRet(p.error, tq.translate("editPost", "Internal database error", "error"), false);
        Result<Post> post = queryOne<Post, Post>(odb::query<Post>::number == p.postNumber
                                                 && odb::query<Post>::board == p.boardName);
        if (post.error)
            return bRet(p.error, tq.translate("editPost", "Internal database error", "error"), false);
        if (!post)
            return bRet(p.error, tq.translate("editPost", "No such post", "error"), false);
        int lvl = registeredUserLevel(hashpass);
        if (lvl < RegisteredUser::ModerLevel && !post->draft())
            return bRet(p.error, tq.translate("editPost", "Not enough rights", "error"), false);
        if (p.password.isEmpty()) {
            if (hashpass != post->hashpass()) {
                if (!moderOnBoard(p.request, board->name()) || registeredUserLevel(post->hashpass()) >= lvl)
                    return bRet(p.error, tq.translate("editPost", "Not enough rights", "error"), false);
            }
        } else if (p.password != post->password()) {
            return bRet(p.error, tq.translate("editPost", "Incorrect password", "error"), false);
        }
        if (p.text.isEmpty() && post->fileInfos().isEmpty())
            return bRet(p.error, tq.translate("editPost", "No text provided", "error"), false);
        if (p.text.length() > int(Tools::maxInfo(Tools::MaxTextFieldLength, p.boardName)))
            return bRet(p.error, tq.translate("editPost", "Text is too long", "error"), false);
        if (p.email.length() > int(Tools::maxInfo(Tools::MaxEmailFieldLength, p.boardName)))
            return bRet(p.error, tq.translate("editPost", "E-mail is too long", "error"), false);
        if (p.name.length() > int(Tools::maxInfo(Tools::MaxNameFieldLength, p.boardName)))
            return bRet(p.error, tq.translate("editPost", "Name is too long", "error"), false);
        if (p.subject.length() > int(Tools::maxInfo(Tools::MaxSubjectFieldLength, p.boardName)))
            return bRet(p.error, tq.translate("editPost", "Subject is too long", "error"), false);
        QString previousText = post->rawText();
        post->setRawText(p.text);
        bool wasDraft = post->draft();
        post->setDraft(board->draftsEnabled() && p.draft);
        if (!post->draft() && !removeFromReferencedPosts(post.data->id(), p.error, tq.locale()))
            return false;
        if (p.raw && lvl >= RegisteredUser::AdminLevel) {
            post->setText(p.text);
            post->setRawHtml(true);
        } else {
            post->setRawHtml(false);
            post->setText(processedText);
            if (!post->draft() && !addToReferencedPosts(post.data, p.referencedPosts, p.error, 0, tq.locale()))
                return false;
        }
        post->setEmail(p.email);
        post->setName(p.name);
        post->setSubject(p.subject);
        Thread thread = *post->thread().load();
        if (post->draft() != wasDraft) {
            if (thread.number() == post->number())
                thread.setDraft(post->draft());
            if (!post->draft())
                post->setDateTime(QDateTime::currentDateTimeUtc());
        }
        if (!wasDraft)
            post->setModificationDateTime(QDateTime::currentDateTimeUtc());
        if (!board->beforeStoringEditedPost(p.request, p.userData, *post, thread, p.error))
            return false;
        t->update(thread);
        update(post);
        t.commit();
        Cache::removePost(p.boardName, p.postNumber);
        Search::removeFromIndex(p.boardName, p.postNumber, previousText);
        Search::addToIndex(p.boardName, p.postNumber, p.text);
        return bRet(p.error, QString(), true);
    } catch (const odb::exception &e) {
        return bRet(p.error, Tools::fromStd(e.what()), false);
    }
}

unsigned int fileCount(const QString &boardName, quint64 postNumber)
{
    if (boardName.isEmpty() || !postNumber)
        return 0;
    try {
        Transaction t;
        if (!t)
            return 0;
        Result<PostId> postId = queryOne<PostId, Post>(odb::query<Post>::board == boardName
                                                       && odb::query<Post>::number == postNumber);
        if (postId.error || !postId)
            return 0;
        Result<FileInfoCount> count = queryOne<FileInfoCount, FileInfo>(odb::query<FileInfo>::post == postId->id);
        if (count.error || !count)
            return 0;
        return count->count;
    } catch (const odb::exception &e) {
        Tools::log("Database::fileCount", e);
        return 0;
    }
}

bool fileExists(const QByteArray &hash, bool *ok)
{
    if (hash.isEmpty())
        return bRet(ok, false, false);
    try {
        Transaction t;
        if (!t)
            return bRet(ok, false, false);
        Result<FileInfoCount> count = queryOne<FileInfoCount, FileInfo>(odb::query<FileInfo>::hash == hash);
        if (count.error)
            return bRet(ok, false, false);
        return bRet(ok, true, count->count > 0);
    } catch (const odb::exception &e) {
        Tools::log("Database::fileExists", e);
        return bRet(ok, false, false);
    }
}

bool fileExists(const QString &hashString, bool *ok)
{
    if (hashString.isEmpty())
        return bRet(ok, false, false);
    bool b = false;
    QByteArray hash = Tools::toHashpass(hashString, &b);
    if (!b || hash.isEmpty())
        return bRet(ok, false, false);
    return fileExists(hash, ok);
}

QList<Post> findPosts(const Search::Query &query, const QString &boardName, bool *ok, QString *error,
                      QString *description, const QLocale &l)
{
    TranslatorQt tq(l);
    bool b = false;
    Search::BoardMap result = Search::find(query, boardName, &b, description, l);
    if (!b)
        return bRet(ok, false, error, tq.translate("findPosts", "Query error", "error"), QList<Post>());
    if (result.isEmpty())
        return bRet(ok, true, error, QString(), description, QString(), QList<Post>());
    try {
        Transaction t;
        if (!t) {
            return bRet(ok, false, error, tq.translate("findPosts", "Internal error", "error"), description,
                        tq.translate("findPosts", "Internal database error", "description"), QList<Post>());
        }
        QString qs;
        for (Search::BoardMap::ConstIterator posts = result.begin(); posts != result.end(); ++posts) {
            qs += "(board = '" + posts.key() + "' AND number IN (";
            foreach (const quint64 &pn, posts.value().keys())
                qs += QString::number(pn) + ", ";
            qs.remove(qs.length() - 2, 2);
            qs += ")) OR ";
        }
        qs.remove(qs.length() - 4, 4);
        return bRet(ok, true, error, QString(), description, QString(), Database::query<Post, Post>(qs));
    } catch (const odb::exception &e) {
        return bRet(ok, false, error, tq.translate("findPosts", "Internal error", "error"), description,
                    Tools::fromStd(e.what()), QList<Post>());
    }
}

void generateRss()
{
    QWriteLocker locker(&rssLock);
    rssMap.clear();
    QString siteProtocol;
    QString siteDomain;
    QString sitePathPrefix;
    {
        SettingsLocker s;
        siteProtocol = s->value("Site/protocol").toString();
        if (siteProtocol.isEmpty())
            siteProtocol = "http";
        siteDomain = s->value("Site/domain").toString();
        sitePathPrefix = s->value("Site/path_prefix").toString();
    }
    QLocale locale = BCoreApplication::locale();
    foreach (const QString &boardName, AbstractBoard::boardNames()) {
        AbstractBoard::LockingWrapper board = AbstractBoard::board(boardName);
        if (board.isNull())
            continue;
        QDomDocument doc;
        doc.appendChild(doc.createProcessingInstruction("xml", "version=\"1.0\""));
        QDomElement root = doc.createElement("rss");
        root.setAttribute("version", "2.0");
        root.setAttribute("xmlns:dc", "http://purl.org/dc/elements/1.1/");
        root.setAttribute("xmlns:atom", "http://www.w3.org/2005/Atom");
        QDomElement chan = doc.createElement("channel");
        QDomElement title = doc.createElement("title");
        title.appendChild(doc.createTextNode(translate("Database::generateRss", "Feed", "channel title") + " "
                                             + siteDomain + "/" + sitePathPrefix + boardName));
        chan.appendChild(title);
        QDomElement link = doc.createElement("link");
        link.appendChild(doc.createTextNode(siteProtocol + "://" + siteDomain + "/" + sitePathPrefix + boardName));
        chan.appendChild(link);
        QDomElement desc = doc.createElement("description");
        desc.appendChild(doc.createTextNode(translate("Database::generateRss", "Last posts from board",
                                                      "channel description") + " /" + boardName + "/"));
        chan.appendChild(desc);
        QDomElement lang = doc.createElement("language");
        lang.appendChild(doc.createTextNode(locale.name().left(2)));
        chan.appendChild(lang);
        QDomElement pub = doc.createElement("pubDate");
        pub.appendChild(doc.createTextNode(QLocale("en_US").toString(QDateTime::currentDateTimeUtc(),
                                                                     "ddd, dd MMM yyyy hh:mm:ss +0000")));
        chan.appendChild(pub);
        QDomElement ttl = doc.createElement("ttl");
        ttl.appendChild(doc.createTextNode("60"));
        chan.appendChild(ttl);
        QDomElement atomLink = doc.createElement("atom:link");
        atomLink.setAttribute("href", siteProtocol + "://" + siteDomain + "/" + sitePathPrefix + boardName
                              + "/rss.xml");
        atomLink.setAttribute("rel", "self");
        atomLink.setAttribute("type", "application/rss+xml");
        chan.appendChild(atomLink);
        try {
            Transaction t;
            if (!t)
                continue;
            QList<Post> list = query<Post, Post>((odb::query<Post>::board == boardName)
                                                 + " ORDER BY dateTime DESC LIMIT 500");
            foreach (const Post &post, list) {
                QDomElement item = doc.createElement("item");
                QDomElement title = doc.createElement("title");
                quint64 threadNumber = post.thread().load()->number();
                bool thread = (post.number() == threadNumber);
                QString t;
                if (thread)
                    t += "[" + translate("Database::generateRss", "New thread", "item title") + "]";
                else
                    t += translate("Database::generateRss", "Reply to thread", "item title") + "";
                t += " ";
                QString subj = post.subject();
                if (subj.isEmpty())
                    subj = post.rawText().left(150);
                if (!subj.isEmpty()) {
                    if (!thread)
                        t += "\"";
                    t += subj;
                    if (!thread)
                        t += "\"";
                } else {
                    t += QString::number(post.number());
                }
                title.appendChild(doc.createCDATASection(t));
                item.appendChild(title);
                QDomElement link = doc.createElement("link");
                QString l = siteProtocol + "://" + siteDomain + "/" + sitePathPrefix + boardName + "/thread/"
                        + QString::number(threadNumber) + ".html";
                link.appendChild(doc.createTextNode(l));
                item.appendChild(link);
                QDomElement desc = doc.createElement("description");
                QString d = "\n";
                typedef QLazyWeakPointer<FileInfo> FileInfoLazy;
                foreach (FileInfoLazy fil, post.fileInfos()) {
                    QSharedPointer<FileInfo> fi = fil.load();
                    d += "<img src=\"" + siteProtocol + "://" + siteDomain + "/" + sitePathPrefix + boardName + "/"
                            + fi->thumbName() + "\"><br />";
                }
                d += post.text();
                d += "\n";
                desc.appendChild(doc.createCDATASection(d));
                item.appendChild(desc);
                QDomElement pub = doc.createElement("pubDate");
                pub.appendChild(doc.createTextNode(QLocale("en_US").toString(post.dateTime(),
                                                                             "ddd, dd MMM yyyy hh:mm:ss +0000")));
                item.appendChild(pub);
                QDomElement guid = doc.createElement("guid");
                l += "#" + QString::number(post.number());
                guid.setAttribute("isPermalink", "ture");
                guid.appendChild(doc.createTextNode(l));
                item.appendChild(guid);
                QDomElement creator = doc.createElement("dc:creator");
                QString c = post.name();
                if (c.isEmpty())
                    c = board->defaultUserName(locale);
                creator.appendChild(doc.createTextNode(c));
                item.appendChild(creator);
                chan.appendChild(item);
            }
        } catch (const odb::exception &e) {
            Tools::log("Database::generateRss", e);
            continue;
        }
        root.appendChild(chan);
        doc.appendChild(root);
        rssMap.insert(boardName, doc.toString(4));
    }
}

GeolocationInfo geolocationInfo(const QString &ip)
{
    GeolocationInfo info;
    info.ip = ip;
    unsigned int n = Tools::ipNum(ip);
    if (!n)
        return info;
    BSqlDatabase db("QSQLITE", "ip2location" + BUuid::createUuid().toString(true));
    db.setDatabaseName(BDirTools::findResource("geolocation/ip2location.sqlite"));
    if (!db.open())
        return info;
    static const QStringList Fields = QStringList() << "country_code" << "country_name" << "city_name";
    BSqlResult r = db.select("ip2location", Fields, BSqlWhere("ip_to >= :ip LIMIT 1", ":ip", n));
    if (!r)
        return info;
    info.cityName = r.value("city_name").toString();
    info.countryCode = r.value("country_code").toString();
    info.countryName = r.value("country_name").toString();
    return info;
}

GeolocationInfo geolocationInfo(const cppcms::http::request &req)
{
    return geolocationInfo(Tools::fromStd(const_cast<cppcms::http::request *>(&req)->remote_addr()));
}

GeolocationInfo geolocationInfo(const QString &boardName, quint64 postNumber)
{
    try {
        Transaction t;
        if (!t)
            return GeolocationInfo();
        Result<Post> post = queryOne<Post, Post>(odb::query<Post>::board == boardName
                                                 && odb::query<Post>::number == postNumber);
        if (post.error || !post)
            return GeolocationInfo();
        GeolocationInfo info;
        info.countryCode = post->countryCode();
        info.countryName = post->countryName();
        info.cityName = post->cityName();
        info.ip = post->posterIp();
        return info;
    }  catch (const odb::exception &e) {
        Tools::log("Database::geolocationInfo", e);
        return GeolocationInfo();
    }
}

QVariant getFileMetaData(const QString &fileName, bool *ok, QString *error, const QLocale &l)
{
    TranslatorQt tq(l);
    if (fileName.isEmpty())
        return bRet(ok, false, error, tq.translate("getFileMetaData", "Invalid file name", "error"), QVariant());
    try {
        Transaction t;
        if (!t) {
            return bRet(ok, false, error, tq.translate("getFileMetaData", "Internal database error", "description"),
                        QVariant());
        }
        Result<FileInfo> fi = queryOne<FileInfo, FileInfo>(odb::query<FileInfo>::name == fileName);
        if (fi.error) {
            return bRet(ok, false, error, tq.translate("getFileMetaData", "Internal database error", "error"),
                        QVariant());
        }
        if (!fi)
            return bRet(ok, false, error, tq.translate("getFileMetaData", "No such file", "error"), QVariant());
        return bRet(ok, true, error, QString(), fi->metaData());
    } catch (const odb::exception &e) {
        return bRet(ok, false, error, Tools::fromStd(e.what()), QVariant());
    }
}

int getNewPostCount(const cppcms::http::request &req, const QString &boardName, quint64 lastPostNumber, bool *ok,
                    QString *error)
{
    AbstractBoard::LockingWrapper board = AbstractBoard::board(boardName);
    TranslatorQt tq(req);
    if (board.isNull())
        return bRet(ok, false, error, tq.translate("getNewPostCount", "Invalid board name", "error"), 0);
    try {
        Transaction t;
        if (!t)
            return bRet(ok, false, error, tq.translate("getNewPostCount", "Internal database error", "error"), 0);
        odb::query<Post> q = odb::query<Post>::board == boardName && odb::query<Post>::draft == false;
        if (lastPostNumber)
            q = q && odb::query<Post>::number > lastPostNumber;
        Result<PostCount> count = queryOne<PostCount, Post>(q);
        if (count.error || !count)
            return bRet(ok, false, error, tq.translate("getNewPostCount", "Internal database error", "error"), 0);
        return bRet(ok, true, error, QString(), count->count);
    }  catch (const odb::exception &e) {
        return bRet(ok, false, error, Tools::fromStd(e.what()), 0);
    }
}

QVariantMap getNewPostCountEx(const cppcms::http::request &req, const QVariantMap &numbers, bool *ok, QString *error)
{
    QStringList boardNames = numbers.keys();
    if (numbers.isEmpty())
        return bRet(ok, true, error, QString(), QVariantMap());
    TranslatorQt tq(req);
    try {
        Transaction t;
        if (!t) {
            return bRet(ok, false, error, tq.translate("getNewPostCountEx", "Internal database error", "error"),
                        QVariantMap());
        }
        QVariantMap m;
        foreach (const QString &bn, boardNames) {
            quint64 lpn = numbers.value(bn).toULongLong();
            odb::query<Post> q = odb::query<Post>::board == bn && odb::query<Post>::draft == false;
            if (lpn)
                q = q && odb::query<Post>::number > lpn;
            Result<PostCount> count = queryOne<PostCount, Post>(q);
            m.insert(bn, count->count);
        }
        return bRet(ok, true, error, QString(), m);
    }  catch (const odb::exception &e) {
        return bRet(ok, false, error, Tools::fromStd(e.what()), QVariantMap());
    }
}

QList<Post> getNewPosts(const cppcms::http::request &req, const QString &boardName, quint64 threadNumber,
                        quint64 lastPostNumber, bool *ok, QString *error)
{
    AbstractBoard::LockingWrapper board = AbstractBoard::board(boardName);
    TranslatorQt tq(req);
    if (board.isNull())
        return bRet(ok, false, error, tq.translate("getNewPosts", "Invalid board name", "error"), QList<Post>());
    if (!threadNumber)
        return bRet(ok, false, error, tq.translate("getNewPosts", "Invalid thread number", "error"), QList<Post>());
    try {
        Transaction t;
        if (!t) {
            return bRet(ok, false, error, tq.translate("getNewPosts", "Internal database error", "error"),
                        QList<Post>());
        }
        odb::query<Thread> q = odb::query<Thread>::board == boardName && odb::query<Thread>::number == threadNumber;
        QByteArray hashpass = Tools::hashpass(req);
        bool modOnBoard = moderOnBoard(req, boardName);
        Result<Thread> thread = queryOne<Thread, Thread>(q);
        if (thread.error) {
            return bRet(ok, false, error, tq.translate("getNewPosts", "Internal database error", "error"),
                        QList<Post>());
        }
        if (!thread)
            return bRet(ok, false, error, tq.translate("getNewPosts", "No such thread", "error"), QList<Post>());
        int lvl = registeredUserLevel(req);
        Post opPost = *thread->posts().first().load();
        if (opPost.draft() && hashpass != opPost.hashpass()
                && (!modOnBoard || registeredUserLevel(opPost.hashpass()) >= lvl)) {
            return bRet(ok, false, error, tq.translate("getNewPosts", "No such thread", "error"), QList<Post>());
        }
        quint64 threadId = thread->id();
        odb::query<Post> qq = odb::query<Post>::board == boardName && odb::query<Post>::thread == threadId;
        if (lastPostNumber)
            qq = qq && odb::query<Post>::number > lastPostNumber;
        QList<Post> posts = query<Post, Post>(qq);
        foreach (int i, bRangeR(posts.size() - 1, 0)) {
            if (posts.at(i).draft() && hashpass != posts.at(i).hashpass()
                    && (!modOnBoard || registeredUserLevel(posts.at(i).hashpass()) >= lvl)) {
                posts.removeAt(i);
            }
        }
        return bRet(ok, true, error, QString(), posts);
    }  catch (const odb::exception &e) {
        return bRet(ok, false, error, Tools::fromStd(e.what()), QList<Post>());
    }
}

Post getPost(const cppcms::http::request &req, const QString &boardName, quint64 postNumber, bool *ok, QString *error)
{
    AbstractBoard::LockingWrapper board = AbstractBoard::board(boardName);
    TranslatorQt tq(req);
    if (board.isNull())
        return bRet(ok, false, error, tq.translate("getPost", "Invalid board name", "error"), Post());
    if (!postNumber)
        return bRet(ok, false, error, tq.translate("getPost", "Invalid post number", "error"), Post());
    try {
        Transaction t;
        if (!t)
            return bRet(ok, false, error, tq.translate("getPost", "Internal database error", "error"), Post());
        odb::query<Post> q = odb::query<Post>::board == boardName && odb::query<Post>::number == postNumber;
        QByteArray hashpass = Tools::hashpass(req);
        bool modOnBoard = moderOnBoard(req, boardName);
        Result<Post> post = queryOne<Post, Post>(q);
        if (post.error)
            return bRet(ok, false, error, tq.translate("getPost", "Internal database error", "error"), Post());
        if (!post)
            return bRet(ok, false, error, tq.translate("getPost", "No such post", "error"), Post());
        if (post->draft() && hashpass != post->hashpass()
                && (!modOnBoard || registeredUserLevel(post->hashpass()) >= registeredUserLevel(req))) {
            return bRet(ok, false, error, tq.translate("getPost", "No such post", "error"), Post());
        }
        return bRet(ok, true, error, QString(), *post);
    }  catch (const odb::exception &e) {
        return bRet(ok, false, error, Tools::fromStd(e.what()), Post());
    }
}

QList<quint64> getThreadNumbers(const cppcms::http::request &req, const QString &boardName, bool *ok, QString *error)
{
    AbstractBoard::LockingWrapper board = AbstractBoard::board(boardName);
    TranslatorQt tq(req);
    if (board.isNull()) {
        return bRet(ok, false, error, tq.translate("getThreadNumbers", "Invalid board name", "error"),
                    QList<quint64>());
    }
    try {
        Transaction t;
        if (!t) {
            return bRet(ok, false, error, tq.translate("getThreadNumbers", "Internal database error", "error"),
                        QList<quint64>());
        }
        QByteArray hashpass = Tools::hashpass(req);
        bool modOnBoard = moderOnBoard(req, boardName);
        int lvl = registeredUserLevel(req);
        QList<Thread> threads = query<Thread, Thread>(odb::query<Thread>::board == boardName);
        QList<quint64> list;
        foreach (Thread t, threads) {
            QSharedPointer<Post> post = t.posts().first().load();
            if (post->draft() && hashpass != post->hashpass()
                    && (!modOnBoard || registeredUserLevel(post->hashpass()) >= lvl)) {
                continue;
            }
            list << t.number();
        }
        return bRet(ok, true, error, QString(), list);
    }  catch (const odb::exception &e) {
        return bRet(ok, false, error, Tools::fromStd(e.what()), QList<quint64>());
    }
}

bool isOp(const QString &boardName, quint64 threadNumber, const QString &userIp, const QByteArray &hashpass)
{
    if (boardName.isEmpty() || !threadNumber)
        return false;
    bool ok = false;
    if (!Tools::ipNum(userIp, &ok) || !ok)
        return false;
    try {
        Transaction t;
        if (!t)
            return false;
        Result<Post> post = queryOne<Post, Post>(odb::query<Post>::board == boardName
                                                 && odb::query<Post>::number == threadNumber);
        if (post.error || !post)
            return false;
        bool b = (post->posterIp() == userIp) || (!hashpass.isEmpty() && post->hashpass() == hashpass);
        return b;
    } catch (const odb::exception &e) {
        Tools::log("Database::isOp", e);
        return false;
    }
}

quint64 lastPostNumber(const QString &boardName, QString *error, const QLocale &l)
{
    TranslatorQt tq(l);
    if (!AbstractBoard::boardNames().contains(boardName))
        return bRet(error, tq.translate("lastPostNumber", "Invalid board name", "error"), 0L);
    try {
        Transaction t;
        if (!t)
            return bRet(error, tq.translate("lastPostNumber", "Invalid database connection", "error"), 0L);
        Result<PostCounter> counter = queryOne<PostCounter, PostCounter>(odb::query<PostCounter>::board == boardName);
        if (!counter && !counter.error) {
            PostCounter c(boardName);
            t->persist(c);
            counter = queryOne<PostCounter, PostCounter>(odb::query<PostCounter>::board == boardName);
        }
        if (counter.error || !counter)
            return bRet(error, tq.translate("incrementPostCounter", "Internal database error", "error"), 0L);
        t.commit();
        return bRet(error, QString(), counter->lastPostNumber());
    } catch (const odb::exception &e) {
        return bRet(error, Tools::fromStd(e.what()), 0L);
    }
}

bool moderOnBoard(const cppcms::http::request &req, const QString &board1, const QString &board2)
{
    QByteArray hp = Tools::hashpass(req);
    if (hp.isEmpty())
        return false;
    return moderOnBoard(hp, board1, board2);
}

bool moderOnBoard(const QByteArray &hashpass, const QString &board1, const QString &board2)
{
    int lvl = registeredUserLevel(hashpass);
    if (lvl < RegisteredUser::ModerLevel)
        return false;
    if (lvl >= RegisteredUser::AdminLevel)
        return true;
    QStringList boards = registeredUserBoards(hashpass);
    return (boards.contains("*") && boards.contains(board1) && (board2.isEmpty() || boards.contains(board2)));
}

quint64 moveThread(const cppcms::http::request &req, const QString &sourceBoard, quint64 threadNumber,
                   const QString &targetBoard, QString *error)
{
    AbstractBoard::LockingWrapper srcBrd = AbstractBoard::board(sourceBoard);
    AbstractBoard::LockingWrapper trgBrd = AbstractBoard::board(targetBoard);
    TranslatorQt tq(req);
    if (srcBrd.isNull() || trgBrd.isNull())
        return bRet(error, tq.translate("Database::moveThread", "Invalid board name", "error"), 0);
    if (!threadNumber)
        return bRet(error, tq.translate("Database::moveThread", "Invalid thread number", "error"), 0);
    if (sourceBoard == targetBoard)
        return bRet(error, tq.translate("Database::moveThread", "Source and target boards are the same", "error"), 0);
    QByteArray hashpass = Tools::hashpass(req);
    if (hashpass.isEmpty())
        return bRet(error, tq.translate("Database::moveThread", "Not logged in", "error"), 0);
    if (!moderOnBoard(hashpass, sourceBoard, targetBoard))
        return bRet(error, tq.translate("Database::moveThread", "Not enough rights", "error"), 0);
    QString storagePath = Tools::storagePath();
    if (storagePath.isEmpty())
        return bRet(error, tq.translate("Database::moveThread", "Internal file system error", "error"), 0);
    QString srcPath = storagePath + "/img/" + sourceBoard;
    QString trgPath = storagePath + "/img/" + targetBoard;
    if (!BDirTools::mkpath(trgPath))
        return bRet(error, tq.translate("Database::moveThread", "Internal file system error", "error"), 0);
    try {
        Transaction t;
        if (!t)
            return bRet(error, tq.translate("Database::moveThread", "Internal database error", "error"), 0);
        Result<Thread> thread = queryOne<Thread, Thread>(odb::query<Thread>::number == threadNumber
                                                 && odb::query<Thread>::board == sourceBoard);
        if (thread.error)
            return bRet(error, tq.translate("Database::moveThread", "Internal database error", "error"), 0);
        if (!thread)
            return bRet(error, tq.translate("Database::moveThread", "No such thread", "error"), false);
        QList<Post> posts = query<Post, Post>(odb::query<Post>::thread == thread->id());
        if (posts.isEmpty())
            return bRet(error, tq.translate("Database::moveThread", "Internal database error", "error"), 0);
        if (posts.first().hashpass() != hashpass
                && registeredUserLevel(posts.first().hashpass()) >= registeredUserLevel(hashpass)) {
            return bRet(error, tq.translate("Database::moveThread", "Not enough rights", "error"), 0);
        }
        quint64 newPostNumber = incrementPostCounter(targetBoard, posts.size(), error, tq.locale());
        if (!newPostNumber)
            return 0;
        quint64 newThreadNumber = newPostNumber - posts.size() + 1;
        QMap<quint64, quint64> oldPostNumbers;
        foreach (int i, bRangeR(posts.size() - 1, 0)) {
            Post &post = posts[i];
            QList<PostReference> referred = query<PostReference, PostReference>(
                        odb::query<PostReference>::targetPost == post.id());
            foreach (int j, bRangeD(0, referred.size() - 1)) {
                PostReference &ref = referred[j];
                QSharedPointer<Post> sourcePost = ref.sourcePost().load();
                if (sourcePost.isNull())
                    return bRet(error, tq.translate("Database::moveThread", "Internal database error", "error"), 0);
                QString text = sourcePost->text();
                QString stns = QString::number(threadNumber);
                QString spns = QString::number(post.number());
                QString ttns = QString::number(newThreadNumber);
                QString tpns= QString::number(newPostNumber);
                QString ns = "<a href=\"/" + targetBoard + "/thread/" + ttns + ".html#" + tpns + "\">&gt;&gt;/"
                        + targetBoard + "/" + tpns + "</a>";
                text.replace("<a href=\"/" + sourceBoard + "/thread/" + stns + ".html#" + spns + "\">&gt;&gt;" + spns
                             + "</a>", ns);
                text.replace("<a href=\"/" + sourceBoard + "/thread/" + stns + ".html#" + spns + "\">&gt;&gt;/"
                             + sourceBoard + "/" + spns + "</a>", ns);
                sourcePost->setText(text);
                t->update(sourcePost);
                Cache::removePost(sourcePost->board(), sourcePost->number());
            }
            QList<FileInfo> fileInfos = query<FileInfo, FileInfo>(odb::query<FileInfo>::post == post.id());
            foreach (int j, bRangeD(0, fileInfos.size() - 1)) {
                FileInfo &fi = fileInfos[j];
                if (!QFile::rename(srcPath + "/" + fi.name(), trgPath + "/" + fi.name()))
                    return bRet(error, tq.translate("Database::moveThread", "Internal file system error", "error"), 0);
                if (!QFile::rename(srcPath + "/" + fi.thumbName(), trgPath + "/" + fi.thumbName()))
                    return bRet(error, tq.translate("Database::moveThread", "Internal file system error", "error"), 0);
            }
            Cache::removePost(post.board(), post.number());
            oldPostNumbers.insert(newPostNumber, post.number());
            post.setNumber(newPostNumber);
            --newPostNumber;
        }
        foreach (int i, bRangeD(0, posts.size() - 1)) {
            Post &post = posts[i];
            QList<PostReference> referenced = query<PostReference, PostReference>(
                        odb::query<PostReference>::sourcePost == post.id());
            foreach (int j, bRangeD(0, referenced.size() - 1)) {
                PostReference &ref = referenced[j];
                QSharedPointer<Post> targetPost = ref.targetPost().load();
                if (targetPost.isNull())
                    return bRet(error, tq.translate("Database::moveThread", "Internal database error", "error"), 0);
                QString text = post.text();
                QString tns = QString::number(targetPost->thread().load()->number());
                QString spns = QString::number(oldPostNumbers.value(targetPost->number()));
                QString tpns = QString::number(targetPost->number());
                text.replace("<a href=\"/" + sourceBoard + "/thread/" + tns + ".html#" + spns + "\">&gt;&gt;" + spns
                             + "</a>", "<a href=\"/" + targetBoard + "/thread/" + tns + ".html#" + tpns + "\">&gt;&gt;"
                             + tpns + "</a>");
                post.setText(text);
            }
            post.setBoard(targetBoard);
            t->update(post);
        }
        thread->setBoard(targetBoard);
        thread->setNumber(newThreadNumber);
        update(thread);
        t.commit();
        return bRet(error, QString(), newThreadNumber);
    } catch (const odb::exception &e) {
        return bRet(error, Tools::fromStd(e.what()), 0);
    }
}

bool postExists(const QString &boardName, quint64 postNumber, quint64 *threadNumber)
{
    QMutexLocker locker(&postMutex);
    try {
        Transaction t;
        if (!t)
            return bRet(threadNumber, quint64(0), false);
        Result<Post> post = queryOne<Post, Post>(odb::query<Post>::board == boardName
                                                 && odb::query<Post>::number == postNumber);
        if (post.error || !post)
            return bRet(threadNumber, quint64(0), false);
        bSet(threadNumber, post->thread().load()->number());
        return true;
    } catch (const odb::exception &e) {
        Tools::log("Database::postExists", e);
        return bRet(threadNumber, quint64(0), false);
    }
}

QString posterIp(const QString &boardName, quint64 postNumber)
{
    try {
        Transaction t;
        if (!t)
            return "";
        Result<Post> post = queryOne<Post, Post>(odb::query<Post>::board == boardName
                                                 && odb::query<Post>::number == postNumber);
        if (post.error || !post)
            return "";
        return post->posterIp();
    }  catch (const odb::exception &e) {
        Tools::log("Database::posterIp", e);
        return "";
    }
}

quint64 postThreadNumber(const QString &boardName, quint64 postNumber)
{
    try {
        Transaction t;
        if (!t)
            return 0;
        Result<Post> post = queryOne<Post, Post>(odb::query<Post>::board == boardName
                                                 && odb::query<Post>::number == postNumber);
        if (post.error || !post)
            return 0;
        return post->thread().load()->number();
    }  catch (const odb::exception &e) {
        Tools::log("Database::postThreadNumber", e);
        return 0;
    }
}

QStringList registeredUserBoards(const cppcms::http::request &req)
{
    QByteArray hp = Tools::hashpass(req);
    if (hp.isEmpty())
        return QStringList();
    return registeredUserBoards(hp);
}

QStringList registeredUserBoards(const QByteArray &hashpass)
{
    bool b = false;
    Tools::toString(hashpass, &b);
    if (!b)
        return QStringList();
    try {
        Transaction t;
        if (!t)
            return QStringList();
        Result<RegisteredUser> user = queryOne<RegisteredUser, RegisteredUser>(
                    odb::query<RegisteredUser>::hashpass == hashpass);
        if (user.error || !user)
            return QStringList();
        return user->boards();
    }  catch (const odb::exception &e) {
        Tools::log("Database::registeredUserBoards", e);
        return QStringList();
    }
}

int registeredUserLevel(const cppcms::http::request &req)
{
    QByteArray hp = Tools::hashpass(req);
    if (hp.isEmpty())
        return -1;
    return registeredUserLevel(hp);
}

int registeredUserLevel(const QByteArray &hashpass)
{
    bool b = false;
    Tools::toString(hashpass, &b);
    if (!b)
        return -1;
    try {
        Transaction t;
        if (!t)
            return -1;
        Result<RegisteredUserLevel> level = queryOne<RegisteredUserLevel, RegisteredUser>(
                    odb::query<RegisteredUser>::hashpass == hashpass);
        if (level.error || !level)
            return -1;
        return level->level;
    }  catch (const odb::exception &e) {
        Tools::log("Database::registeredUserLevel", e);
        return -1;
    }
}

bool registerUser(const QByteArray &hashpass, RegisteredUser::Level level, const QStringList &boards, QString *error,
                  const QLocale &l)
{
    bool b = false;
    Tools::toString(hashpass, &b);
    TranslatorQt tq(l);
    if (!b)
        return bRet(error, tq.translate("registerUser", "Invalid hashpass", "error"), false);
    QStringList boardNames = AbstractBoard::boardNames();
    foreach (const QString &bn, boardNames) {
        if ("*" != bn && !boardNames.contains(bn))
            return bRet(error, tq.translate("registerUser", "Invalid board(s)", "error"), false);
    }
    try {
        Transaction t;
        if (!t)
            return bRet(error, tq.translate("registerUser", "Internal database error", "error"), false);
        RegisteredUser user(hashpass, QDateTime::currentDateTimeUtc(), level, boards);
        t->persist(user);
        return bRet(error, QString(), true);
    } catch (const odb::exception &e) {
        return bRet(error, Tools::fromStd(e.what()), false);
    }
}

int rerenderPosts(const QStringList boardNames, QString *error, const QLocale &l)
{
    TranslatorQt tq(l);
    QWriteLocker locker(&processTextLock);
    QMap<quint64, PostTmpInfo> postIds;
    try {
        Transaction t;
        if (!t)
            return bRet(error, tq.translate("rerenderPosts", "Internal database error", "error"), -1);
        odb::query<Post> q = (odb::query<Post>::rawHtml == false);
        if (!boardNames.isEmpty()) {
            odb::query<Post> qq = (odb::query<Post>::board == boardNames.first());
            foreach (const QString &board, boardNames.mid(1))
                qq = qq || (odb::query<Post>::board == board);
            q = q && qq;
        }
        QList<PostIdBoardRawText> ids = query<PostIdBoardRawText, Post>(q);
        foreach (const PostIdBoardRawText &id, ids) {
            PostTmpInfo tmp;
            tmp.board = id.board;
            tmp.text = id.rawText;
            postIds.insert(id.id, tmp);
        }
        t.commit();
    } catch (const odb::exception &e) {
        return bRet(error, Tools::fromStd(e.what()), -1);
    }
    if (postIds.isEmpty())
        return bRet(error, QString(), 0);
    int sz = postIds.keys().size();
    int curr = 1;
    foreach (quint64 id, postIds.keys()) {
        bWriteLine(QString::number(curr) + "/" + QString::number(sz));
        ++curr;
        PostTmpInfo &tmp = postIds[id];
        tmp.text = Controller::processPostText(tmp.text, tmp.board, &tmp.refs);
    }
    int count = 0;
    int offset = 0;
    while (offset < postIds.size()) {
        try {
            Transaction t;
            if (!t)
                return bRet(error, tq.translate("rerenderPosts", "Internal database error", "error"), -1);
            QString qs = "id IN (";
            static const int Offset = 100;
            foreach (quint64 id, postIds.keys().mid(count, Offset))
                qs += QString::number(id) + ", ";
            qs.remove(qs.length() - 2, 2);
            qs += ")";
            QList<Post> posts = query<Post, Post>(qs);
            foreach (int i, bRangeD(0, posts.size() - 1)) {
                Post &post = posts[i];
                if (!post.draft() && !removeFromReferencedPosts(post.id(), error, tq.locale()))
                    return -1;
                if (post.rawHtml())
                    continue;
                const PostTmpInfo &tmp = postIds.value(post.id());
                post.setText(tmp.text);
                QSharedPointer<Post> sp(new Post(post));
                if (!post.draft() && !addToReferencedPosts(sp, tmp.refs, error, 0, tq.locale()))
                    return -1;
                t->update(post);
                Cache::removePost(post.board(), post.number());
            }
            t.commit();
            count += posts.size();
            offset += Offset;
        } catch (const odb::exception &e) {
            return bRet(error, Tools::fromStd(e.what()), -1);
        }
    }
    return bRet(error, QString(), count);
}

QString rss(const QString &boardName)
{
    QReadLocker locker(&rssLock);
    return rssMap.value(boardName);
}

bool setThreadFixed(const QString &board, quint64 threadNumber, bool fixed, QString *error, const QLocale &l)
{
    return setThreadFixedInternal(board, threadNumber, fixed, error, l);
}

bool setThreadFixed(const QString &boardName, quint64 threadNumber, bool fixed, const cppcms::http::request &req,
                    QString *error)
{
    TranslatorQt tq(req);
    if (!AbstractBoard::boardNames().contains(boardName))
        return bRet(error, tq.translate("setThreadFixed", "Invalid board name", "error"), false);
    if (!threadNumber)
        return bRet(error, tq.translate("setThreadFixed", "Invalid thread number", "error"), false);
    QByteArray hashpass = Tools::hashpass(req);
    if (hashpass.isEmpty())
        return bRet(error, tq.translate("setThreadFixed", "Not logged in", "error"), false);
    try {
        Transaction t;
        if (!t)
            return bRet(error, tq.translate("setThreadFixed", "Internal database error", "error"), false);
        if (!moderOnBoard(req, boardName))
            return bRet(error, tq.translate("setThreadFixed", "Not enough rights", "error"), false);
        if (!setThreadFixedInternal(boardName, threadNumber, fixed, error, tq.locale()))
            return false;
        t.commit();
        return bRet(error, QString(), true);
    } catch (const odb::exception &e) {
        return bRet(error, Tools::fromStd(e.what()), false);
    }
}

bool setThreadOpened(const QString &board, quint64 threadNumber, bool opened, QString *error, const QLocale &l)
{
    return setThreadOpenedInternal(board, threadNumber, opened, error, l);
}

bool setThreadOpened(const QString &boardName, quint64 threadNumber, bool opened, const cppcms::http::request &req,
                     QString *error)
{
    TranslatorQt tq(req);
    if (!AbstractBoard::boardNames().contains(boardName))
        return bRet(error, tq.translate("setThreadOpened", "Invalid board name", "error"), false);
    if (!threadNumber)
        return bRet(error, tq.translate("setThreadOpened", "Invalid thread number", "error"), false);
    QByteArray hashpass = Tools::hashpass(req);
    if (hashpass.isEmpty())
        return bRet(error, tq.translate("setThreadOpened", "Not logged in", "error"), false);
    try {
        Transaction t;
        if (!t)
            return bRet(error, tq.translate("setThreadOpened", "Internal database error", "error"), false);
        if (!moderOnBoard(req, boardName))
            return bRet(error, tq.translate("setThreadOpened", "Not enough rights", "error"), false);
        if (!setThreadOpenedInternal(boardName, threadNumber, opened, error, tq.locale()))
            return false;
        t.commit();
        return bRet(error, QString(), true);
    } catch (const odb::exception &e) {
        return bRet(error, Tools::fromStd(e.what()), false);
    }
}

bool setVoteOpened(quint64 postNumber, bool opened, const QByteArray &password, const cppcms::http::request &req,
                   QString *error)
{
    TranslatorQt tq(req);
    if (!postNumber)
        return bRet(error, tq.translate("setVoteOpened", "Invalid post number", "error"), false);
    QByteArray hashpass = Tools::hashpass(req);
    if (password.isEmpty() && hashpass.isEmpty())
        return bRet(error, tq.translate("setVoteOpened", "Invalid password", "error"), false);
    try {
        Transaction t;
        if (!t)
            return bRet(error, tq.translate("setVoteOpened", "Internal database error", "error"), false);
        Result<Post> post = queryOne<Post, Post>(odb::query<Post>::board == "rpg"
                                                 && odb::query<Post>::number == postNumber);
        if (post.error)
            return bRet(error, tq.translate("setVoteOpened", "Internal database error", "error"), false);
        if (!post)
            return bRet(error, tq.translate("setVoteOpened", "No such post", "error"), false);
        if (password.isEmpty()) {
            if (hashpass != post->hashpass()) {
                int lvl = registeredUserLevel(req);
                if (!moderOnBoard(req, "rpg") || registeredUserLevel(post->hashpass()) >= lvl)
                    return bRet(error, tq.translate("setVoteOpened", "Not enough rights", "error"), false);
            }
        } else if (password != post->password()) {
            return bRet(error, tq.translate("setVoteOpened", "Incorrect password", "error"), false);
        }
        QVariantMap m = post->userData().toMap();
        if (m.value("disabled") == !opened)
            return bRet(error, QString(), true);
        m["disabled"] = !opened;
        post->setUserData(m);
        update(post);
        t.commit();
        Cache::removePost("rpg", postNumber);
        return bRet(error, QString(), true);
    } catch (const odb::exception &e) {
        return bRet(error, Tools::fromStd(e.what()), false);
    }
}

bool unvote(quint64 postNumber, const cppcms::http::request &req, QString *error)
{
    TranslatorQt tq(req);
    if (!postNumber)
        return bRet(error, tq.translate("unvote", "Invalid post number", "error"), false);
    try {
        Transaction t;
        if (!t)
            return bRet(error, tq.translate("unvote", "Internal database error", "error"), false);
        Result<Post> post = queryOne<Post, Post>(odb::query<Post>::board == "rpg"
                                                 && odb::query<Post>::number == postNumber);
        if (post.error)
            return bRet(error, tq.translate("unvote", "Internal database error", "error"), false);
        if (!post)
            return bRet(error, tq.translate("unvote", "No such post", "error"), false);
        QVariantMap m = post->userData().toMap();
        if (m.value("disabled").toBool())
            return bRet(error, tq.translate("unvote", "Voting disabled", "error"), false);
        unsigned int ip = Tools::ipNum(Tools::userIp(req));
        QVariantList users = m.value("users").toList();
        bool voted = false;
        foreach (int i, bRangeR(users.size() - 1, 0)) {
            if (users.at(i).toUInt() == ip) {
                users.removeAt(i);
                voted = true;
                break;
            }
        }
        if (!voted)
            return bRet(error, tq.translate("unvote", "Not voted yet", "error"), false);
        m["users"] = users;
        QVariantList variants = m.value("variants").toList();
        foreach (int i, bRangeD(0, variants.size() - 1)) {
            QVariantMap mm = variants.at(i).toMap();
            QVariantList list = mm["users"].toList();
            foreach (int i, bRangeR(list.size() - 1, 0)) {
                if (list.at(i).toUInt() == ip) {
                    list.removeAt(i);
                    mm["voteCount"] = mm.value("voteCount").toUInt() - 1;
                    break;
                }
            }
            mm["users"] = list;
            variants[i] = mm;
        }
        m["variants"] = variants;
        post->setUserData(m);
        update(post);
        Cache::removePost("rpg", postNumber);
        t.commit();
        return bRet(error, QString(), true);
    } catch (const odb::exception &e) {
        return bRet(error, Tools::fromStd(e.what()), false);
    }
}

BanInfo userBanInfo(const QString &ip, const QString &boardName, bool *ok, QString *error, const QLocale &l)
{
    BanInfo inf;
    inf.level = 0;
    TranslatorQt tq(l);
    if (ip.isEmpty())
        return bRet(ok, false, error, tq.translate("userBanInfo", "Internal logic error", "description"), inf);
    try {
        Transaction t;
        if (!t)
            return bRet(ok, false, error, tq.translate("userBanInfo", "Internal database error", "description"), inf);
        QList<BannedUser> list = query<BannedUser, BannedUser>(odb::query<BannedUser>::board == "*"
                                                               && odb::query<BannedUser>::ip == ip);
        foreach (const BannedUser &u, list) {
            QDateTime expires = u.expirationDateTime().toUTC();
            if (!expires.isValid() || expires > QDateTime::currentDateTimeUtc()) {
                inf.level = u.level();
                inf.boardName = u.board();
                inf.dateTime = u.dateTime();
                inf.reason = u.reason();
                inf.expires = expires;
                break;
            }
        }
        if (inf.level <= 0 && !boardName.isEmpty()) {
            list = query<BannedUser, BannedUser>(odb::query<BannedUser>::board == boardName
                                                 && odb::query<BannedUser>::ip == ip);
            foreach (const BannedUser &u, list) {
                QDateTime expires = u.expirationDateTime().toUTC();
                if (!expires.isValid() || expires > QDateTime::currentDateTimeUtc()) {
                    inf.level = u.level();
                    inf.boardName = u.board();
                    inf.dateTime = u.dateTime();
                    inf.reason = u.reason();
                    inf.expires = expires;
                    break;
                }
            }
        }
        return bRet(ok, true, error, QString(), inf);
    }  catch (const odb::exception &e) {
        return bRet(ok, false, error, Tools::fromStd(e.what()), inf);
    }
}

bool vote(quint64 postNumber, const QStringList &votes, const cppcms::http::request &req, QString *error)
{
    TranslatorQt tq(req);
    if (!postNumber)
        return bRet(error, tq.translate("vote", "Invalid post number", "error"), false);
    if (votes.isEmpty())
        return bRet(error, tq.translate("vote", "No votes", "error"), false);
    try {
        Transaction t;
        if (!t)
            return bRet(error, tq.translate("vote", "Internal database error", "error"), false);
        Result<Post> post = queryOne<Post, Post>(odb::query<Post>::board == "rpg"
                                                 && odb::query<Post>::number == postNumber);
        if (post.error)
            return bRet(error, tq.translate("vote", "Internal database error", "error"), false);
        if (!post)
            return bRet(error, tq.translate("vote", "No such post", "error"), false);
        QString userIp = Tools::userIp(req);
        if (post->posterIp() == userIp)
            return bRet(error, tq.translate("vote", "Attempt to vote in an own voting", "error"), false);
        QVariantMap m = post->userData().toMap();
        if (m.value("disabled").toBool())
            return bRet(error, tq.translate("vote", "Voting disabled", "error"), false);
        if (!m.value("multiple").toBool() && votes.size() > 1)
            return bRet(error, tq.translate("vote", "Too many votes", "error"), false);
        unsigned int ip = Tools::ipNum(userIp);
        QVariantList users = m.value("users").toList();
        foreach (const QVariant &v, users) {
            if (v.toUInt() == ip)
                return bRet(error, tq.translate("vote", "Repeated voting", "error"), false);
        }
        users << ip;
        m["users"] = users;
        QVariantList variants = m.value("variants").toList();
        foreach (const QString &id, votes) {
            if (votes.count(id) > 1)
                return bRet(error, tq.translate("vote", "Multiple vote occurance", "error"), false);
            bool found = false;
            for (int i = 0; i < variants.size(); ++i) {
                QVariantMap mm = variants.at(i).toMap();
                if (mm.value("id").toString() != id)
                    continue;
                mm["voteCount"] = mm.value("voteCount").toUInt() + 1;
                QVariantList list = mm["users"].toList();
                list << ip;
                mm["users"] = list;
                variants[i] = mm;
                found = true;
            }
            if (!found)
                return bRet(error, tq.translate("vote", "Invalid vote", "error"), false);
        }
        m["variants"] = variants;
        post->setUserData(m);
        update(post);
        Cache::removePost("rpg", postNumber);
        t.commit();
        return bRet(error, QString(), true);
    } catch (const odb::exception &e) {
        return bRet(error, Tools::fromStd(e.what()), false);
    }
}

}
