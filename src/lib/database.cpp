#include "database.h"

#include "board/abstractboard.h"
#include "cache.h"
#include "controller/controller.h"
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
#include <BTextTools>

#include <QByteArray>
#include <QCryptographicHash>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QScopedPointer>
#include <QSharedPointer>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QVariantMap>

#include <cppcms/http_request.h>

#include <odb/connection.hxx>
#include <odb/database.hxx>
#include <odb/query.hxx>
#include <odb/schema-catalog.hxx>
#include <odb/sqlite/database.hxx>
#include <odb/transaction.hxx>

namespace Database
{

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

FindPostsParameters::FindPostsParameters(const QLocale &l) :
    locale(l)
{
    ok = 0;
    error = 0;
    description = 0;
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
    }
};

static bool addToReferencedPosts(QSharedPointer<Post> post, const RefMap &referencedPosts, QString *error = 0,
                                 QString *description = 0)
{
    TranslatorQt tq;
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

static bool removeFromReferencedPosts(quint64 postId, QString *error = 0)
{
    TranslatorQt tq;
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
        if (!QFile::copy(sfn, fn) || !QFile::copy(spfn, pfn)) {
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
        qDebug() << Tools::fromStd(e.what());
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
                      AbstractBoard::FileTransaction &ft, bool thread, QString *error = 0, QString *description = 0,
                      const QLocale &l = BCoreApplication::locale())
{
    TranslatorQt tq(l);
    Tools::Post post = Tools::toPost(params, files);
    if (thread && post.files.isEmpty() && post.fileHashes.isEmpty()) {
        return bRet(error, tq.translate("saveFiles", "No file", "error"), description,
                    tq.translate("saveFiles", "Attempt to create a thread without attaching a file", "description"),
                    false);
    }
    if (post.text.isEmpty() && post.files.isEmpty() && post.fileHashes.isEmpty()) {
        return bRet(error, tq.translate("saveFiles", "No file/text", "error"), description,
                    tq.translate("saveFiles", "Both file and comment are missing", "description"), false);
    }
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

static bool testCaptcha(const cppcms::http::request &req, const QMap<QString, QString> &params, QString *error = 0,
                        QString *description = 0, const QLocale &l = BCoreApplication::locale())
{
    TranslatorQt tq(l);
    AbstractBoard *board = AbstractBoard::board(params.value("board"));
    if (!board) {
        return bRet(error, tq.translate("testCaptcha", "Internal error", "error"), description,
                    tq.translate("testCaptcha", "Internal logic error", "description"), false);
    }
    if (!Tools::captchaEnabled(board->name()))
        return bRet(error, QString(), description, QString(), true);
    QString ip = Tools::userIp(req);
    if (board->captchaQuota(ip)) {
        board->captchaUsed(ip);
    } else {
        QString err;
        if (!board->isCaptchaValid(req, params, err))
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
    AbstractBoard *board = AbstractBoard::board(boardName);
    TranslatorQt tq(p.locale);
    bSet(p.postNumber, quint64(0L));
    if (!board) {
        return bRet(p.error, tq.translate("createPostInternal", "Internal error", "error"), p.description,
                           tq.translate("createPostInternal", "Internal logic error", "description"), false);
    }
    bool isThread = p.threadNumber;
    if (!p.threadNumber)
        p.threadNumber = p.params.value("thread").toULongLong();
    Tools::Post post = Tools::toPost(p.params, p.files);
    try {
        Transaction t;
        if (!t) {
            return bRet(p.error, tq.translate("createPostInternal", "Internal error", "error"), p.description,
                        tq.translate("createPostInternal", "Internal database error", "description"), false);
        }
        QString err;
        quint64 postNumber = p.dateTime.isValid() ? lastPostNumber(boardName, &err, tq.locale())
                                                  : incrementPostCounter(boardName, &err, tq.locale());
        if (!postNumber) {
            return bRet(p.error, tq.translate("createPostInternal", "Internal error", "error"), p.description, err,
                        false);
        }
        Result<Thread> thread = queryOne<Thread, Thread>(odb::query<Thread>::number == p.threadNumber
                                                         && odb::query<Thread>::board == boardName);
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
        QSharedPointer<Post> ps(new Post(boardName, postNumber, p.dateTime, thread.data, ip, post.password, hp));
        ps->setEmail(post.email);
        ps->setName(post.name);
        ps->setSubject(post.subject);
        ps->setRawText(post.text);
        if (board->draftsEnabled() && post.draft)
            ps->setDraft(true);
        bool raw = post.raw && registeredUserLevel(p.request) >= RegisteredUser::AdminLevel;
        RefMap refs;
        if (raw) {
            ps->setText(post.text);
            ps->setRawHtml(true);
        } else {
            ps->setText(Controller::processPostText(post.text, boardName, !ps->draft() ? &refs : 0));
            bSet(p.referencedPosts, refs);
        }
        ps->setShowTripcode(!Tools::cookieValue(p.request, "show_tripcode").compare("true", Qt::CaseInsensitive));
        if (bump) {
            thread->setDateTime(p.dateTime);
            update(thread);
        }
        board->beforeStoring(ps.data(), p.params, isThread);
        t->persist(ps);
        foreach (const AbstractBoard::FileInfo &fi, p.fileTransaction.fileInfos()) {
            FileInfo fileInfo(fi.name, fi.hash, fi.mimeType, fi.size, fi.height, fi.width, fi.thumbName,
                              fi.thumbHeight, fi.thumbWidth, ps);
            t->persist(fileInfo);
        }
        if (!raw && !ps->draft() && !addToReferencedPosts(ps, refs, p.error, p.description))
            return false;
        bSet(p.postNumber, postNumber);
        t.commit();
        p.fileTransaction.commit();
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
    AbstractBoard *board = AbstractBoard::board(boardName);
    if (!board)
        return bRet(error, tq.translate("deletePostInternal", "Internal logic error", "error"), false);
    if (!postNumber)
        return bRet(error, tq.translate("deletePostInternal", "Invalid post number", "error"), false);
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
        if (!removeFromReferencedPosts(post.data->id(), error))
            return false;
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
            }
            t->erase_query<Post>(odb::query<Post>::thread == thread->id());
            t->erase_query<Thread>(odb::query<Thread>::id == thread->id());
        } else {
            bool ok = false;
            filesToDelete << deleteFileInfos(*post, &ok);
            if (!ok)
                return bRet(error, tq.translate("deletePostInternal", "Internal database error", "error"), false);
            t->erase_query<Post>(odb::query<Post>::board == boardName && odb::query<Post>::number == postNumber);
        }
        foreach (Post p, referenced) {
            p.setText(Controller::processPostText(p.rawText(), p.board()));
            Cache::removePost(p.board(), p.number());
            t->update(p);
        }
        Cache::removePost(boardName, postNumber);
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
        Result<Thread> thread = queryOne<Thread, Thread>(odb::query<Thread>::number == threadNumber
                                                         && odb::query<Thread>::board == board);
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
        Result<Thread> thread = queryOne<Thread, Thread>(odb::query<Thread>::number == threadNumber
                                                         && odb::query<Thread>::board == board);
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
    }  catch (const odb::exception &e) {
        qDebug() << e.what();
    }
}

bool createPost(CreatePostParameters &p, quint64 *postNumber)
{
    bSet(postNumber, quint64(0L));
    if (!testCaptcha(p.request, p.params, p.error, p.description, p.locale))
        return false;
    TranslatorQt tq(p.locale);
    AbstractBoard *board = AbstractBoard::board(p.params.value("board"));
    if (!board) {
        return bRet(p.error, tq.translate("createPost", "Internal error", "error"), p.description,
                    tq.translate("createPost", "Internal logic error", "description"), false);
    }
    CreatePostInternalParameters pp(p, board);
    if (!saveFiles(p.params, p.files, pp.fileTransaction, false, p.error, p.description, p.locale))
        return false;
    try {
        Transaction t;
        pp.postNumber = postNumber;
        if (!createPostInternal(pp))
            return false;
        t.commit();
        return bRet(p.error, QString(), p.description, QString(), true);
    } catch (const odb::exception &e) {
        return bRet(p.error, tq.translate("createPost", "Internal error", "error"), p.description,
                    Tools::fromStd(e.what()), false);
    }
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
        qDebug() << e.what();
    }
}

quint64 createThread(CreateThreadParameters &p)
{
    if (!testCaptcha(p.request, p.params, p.error, p.description, p.locale))
        return false;
    QString boardName = p.params.value("board");
    AbstractBoard *board = AbstractBoard::board(boardName);
    TranslatorQt tq(p.locale);
    if (!board) {
        return bRet(p.error, tq.translate("createThread", "Internal error", "error"), p.description,
                    tq.translate("createThread", "Internal logic error", "description"), 0L);
    }
    CreatePostInternalParameters pp(p, board);
    if (!saveFiles(p.params, p.files, pp.fileTransaction, true, p.error, p.description, p.locale))
        return false;
    try {
        QStringList filesToDelete;
        Transaction t;
        QString err;
        quint64 postNumber = incrementPostCounter(p.params.value("board"), &err, p.locale);
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
        deleteFiles(boardName, QStringList() << fileName);
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

bool deletePost(const QString &boardName, quint64 postNumber,  const cppcms::http::request &req,
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
    try {
        QStringList filesToDelete;
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
        if (!deletePostInternal(boardName, postNumber, error, tq.locale(), filesToDelete))
            return false;
        t.commit();
        deleteFiles(boardName, filesToDelete);
        return bRet(error, QString(), true);
    } catch (const odb::exception &e) {
        return bRet(error, Tools::fromStd(e.what()), false);
    }
}

bool editPost(EditPostParameters &p)
{
    AbstractBoard *board = AbstractBoard::board(p.boardName);
    TranslatorQt tq(p.request);
    if (!board)
        return bRet(p.error, tq.translate("editPost", "Invalid board name", "error"), false);
    if (!p.postNumber)
        return bRet(p.error, tq.translate("editPost", "Invalid post number", "error"), false);
    QByteArray hashpass = Tools::hashpass(p.request);
    if (p.password.isEmpty() && hashpass.isEmpty())
        return bRet(p.error, tq.translate("editPost", "Invalid password", "error"), false);
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
        post->setRawText(p.text);
        bool wasDraft = post->draft();
        post->setDraft(board->draftsEnabled() && p.draft);
        if (!post->draft() && !removeFromReferencedPosts(post.data->id(), p.error))
            return false;
        if (p.raw && lvl >= RegisteredUser::AdminLevel) {
            post->setText(p.text);
            post->setRawHtml(true);
        } else {
            post->setRawHtml(false);
            post->setText(Controller::processPostText(p.text, p.boardName, !post->draft() ? &p.referencedPosts : 0));
            if (!post->draft() && !addToReferencedPosts(post.data, p.referencedPosts, p.error))
                return false;
        }
        post->setEmail(p.email);
        post->setName(p.name);
        post->setSubject(p.subject);
        bool draftLast = post->draft();
        Thread thread = *post->thread().load();
        if (post->draft() != draftLast) {
            if (thread.number() == post->number())
                thread.setDraft(post->draft());
        }
        if (!wasDraft)
            post->setModificationDateTime(QDateTime::currentDateTimeUtc());
        if (!board->editPost(p.request, p.userData, *post, thread, p.error))
            return false;
        t->update(thread);
        update(post);
        Cache::removePost(p.boardName, p.postNumber);
        t.commit();
        return bRet(p.error, QString(), true);
    } catch (const odb::exception &e) {
        return bRet(p.error, Tools::fromStd(e.what()), false);
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
        t.commit();
        return bRet(ok, true, count->count > 0);
    } catch (const odb::exception &e) {
        qDebug() << Tools::fromStd(e.what());
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

QList<Post> findPosts(FindPostsParameters &p)
{
    TranslatorQt tq(p.locale);
    if (p.possiblePhrases.isEmpty() && p.requiredPhrases.isEmpty() && p.excludedPhrases.isEmpty()) {
        return bRet(p.ok, false, p.error, tq.translate("findPosts", "Invalid parameters", "error"), p.description,
                    tq.translate("findPosts", "No phrases to search for", "description"), QList<Post>());
    }
    try {
        Transaction t;
        if (!t) {
            return bRet(p.ok, false, p.error, tq.translate("findPosts", "Internal error", "error"), p.description,
                        tq.translate("findPosts", "Internal database error", "description"), QList<Post>());
        }
        odb::query<Post> q;
        QString qs;
        if (!p.boardNames.isEmpty()) {
            qs += "(";
            foreach (const QString &bn, p.boardNames)
                qs += "board = '" + bn + "' OR ";
            qs.remove(qs.length() - 4, 4);
            qs += ") AND ";
        }
        foreach (const QString &s, p.requiredPhrases)
            qs += "rawText LIKE '%" + BTextTools::unwrapped(s, "\"").replace('%', "\\%") + "%' AND ";
        foreach (const QString &s, p.excludedPhrases)
            qs += "rawText NOT LIKE '%" + BTextTools::unwrapped(s, "\"").replace('%', "\\%") + "%' AND ";
        if (!p.possiblePhrases.isEmpty()) {
            qs += "(";
            foreach (const QString &s, p.possiblePhrases)
                qs += "rawText LIKE '%" + BTextTools::unwrapped(s, "\"").replace('%', "\\%") + "%' OR ";
            qs.remove(qs.length() - 4, 4);
            qs += ")";
        }
        if (qs.endsWith(" AND "))
            qs.remove(qs.length() - 5, 5);
        q = q + Tools::toStd(qs).data();
        return bRet(p.ok, true, p.error, QString(), p.description, QString(), query<Post, Post>(q));
    } catch (const odb::exception &e) {
        return bRet(p.ok, false, p.error, tq.translate("findPosts", "Internal error", "error"), p.description,
                    Tools::fromStd(e.what()), QList<Post>());
    }
}

QList<Post> getNewPosts(const cppcms::http::request &req, const QString &boardName, quint64 threadNumber,
                        quint64 lastPostNumber, bool *ok, QString *error)
{
    AbstractBoard *board = AbstractBoard::board(boardName);
    TranslatorQt tq(req);
    if (!board)
        return bRet(ok, false, error, tq.translate("getNewPosts", "Invalid board name", "error"), QList<Post>());
    if (!threadNumber)
        return bRet(ok, false, error, tq.translate("getNewPosts", "Invalid thread number", "error"), QList<Post>());
    if (!lastPostNumber)
        return bRet(ok, false, error, tq.translate("getNewPosts", "Invalid post number", "error"), QList<Post>());
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
        odb::query<Post> qq = odb::query<Post>::board == boardName && odb::query<Post>::thread == threadId
                && odb::query<Post>::number > lastPostNumber;
        QList<Post> posts = query<Post, Post>(qq);
        foreach (int i, bRangeR(posts.size() - 1, 0)) {
            if (posts.at(i).draft() && hashpass != posts.at(i).hashpass()
                    && (!modOnBoard || registeredUserLevel(posts.at(i).hashpass()) >= lvl)) {
                posts.removeAt(i);
            }
        }
        t.commit();
        return bRet(ok, true, error, QString(), posts);
    }  catch (const odb::exception &e) {
        return bRet(ok, false, error, Tools::fromStd(e.what()), QList<Post>());
    }
}

Post getPost(const cppcms::http::request &req, const QString &boardName, quint64 postNumber, bool *ok, QString *error)
{
    AbstractBoard *board = AbstractBoard::board(boardName);
    TranslatorQt tq(req);
    if (!board)
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
        t.commit();
        return bRet(ok, true, error, QString(), *post);
    }  catch (const odb::exception &e) {
        return bRet(ok, false, error, Tools::fromStd(e.what()), Post());
    }
}

quint64 incrementPostCounter(const QString &boardName, QString *error, const QLocale &l)
{
    TranslatorQt tq(l);
    if (!AbstractBoard::boardNames().contains(boardName))
        return bRet(error, tq.translate("incrementPostCounter", "Invalid board name", "error"), 0L);
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
        incremented = counter->incrementLastPostNumber();
        update(counter);
        t.commit();
    } catch (const odb::exception &e) {
        return bRet(error, Tools::fromStd(e.what()), 0L);
    }
    return bRet(error, QString(), incremented);
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

bool postExists(const QString &boardName, quint64 postNumber, quint64 *threadNumber)
{
    try {
        Transaction t;
        if (!t)
            return bRet(threadNumber, quint64(0), false);
        Result<Post> post = queryOne<Post, Post>(odb::query<Post>::board == boardName
                                                 && odb::query<Post>::number == postNumber);
        if (post.error || !post)
            return bRet(threadNumber, quint64(0), false);
        bSet(threadNumber, post->thread().load()->number());
        t.commit();
        return true;
    }  catch (const odb::exception &e) {
        qDebug() << e.what();
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
        t.commit();
        return post->posterIp();
    }  catch (const odb::exception &e) {
        qDebug() << e.what();
        return "";
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
        t.commit();
        return user->boards();
    }  catch (const odb::exception &e) {
        qDebug() << e.what();
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
        t.commit();
        return level->level;
    }  catch (const odb::exception &e) {
        qDebug() << e.what();
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
        t.commit();
        return bRet(error, QString(), true);
    } catch (const odb::exception &e) {
        return bRet(error, Tools::fromStd(e.what()), false);
    }
}

int rerenderPosts(const QStringList boardNames, QString *error, const QLocale &l)
{
    TranslatorQt tq(l);
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
        QList<Post> posts = query<Post, Post>(q);
        foreach (int i, bRangeD(0, posts.size() - 1)) {
            Post &post = posts[i];
            if (!post.draft() && !removeFromReferencedPosts(post.id(), error))
                return -1;
            RefMap refs;
            post.setText(Controller::processPostText(post.rawText(), post.board(), &refs));
            QSharedPointer<Post> sp(new Post(post));
            if (!post.draft() && !addToReferencedPosts(sp, refs, error))
                return -1;
            t->update(post);
            Cache::removePost(post.board(), post.number());
        }
        t.commit();
        return bRet(error, QString(), posts.size());
    } catch (const odb::exception &e) {
        return bRet(error, Tools::fromStd(e.what()), -1);
    }
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

BanInfo userBanInfo(const QString &ip, const QString &boardName, bool *ok, QString *error, const QLocale &l)
{
    BanInfo inf;
    inf.level = 0;
    TranslatorQt tq(l);
    if (ip.isEmpty())
        return bRet(ok, false, error, tq.translate("userBanInfo", "Internal logic error", "description"), inf);
    try {
        Transaction t;
        if (!t) {
            return bRet(ok, false, error, tq.translate("userBanInfo", "Internal database error", "description"), inf);
        }
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
        t.commit();
        return bRet(ok, true, error, QString(), inf);
    }  catch (const odb::exception &e) {
        return bRet(ok, false, error, Tools::fromStd(e.what()), inf);
    }
}

}
