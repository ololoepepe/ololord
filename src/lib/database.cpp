#include "database.h"

#include "board/abstractboard.h"
#include "cache.h"
#include "controller/controller.h"
#include "stored/banneduser.h"
#include "stored/banneduser-odb.hxx"
#include "stored/filehash.h"
#include "stored/filehash-odb.hxx"
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

class FileTransaction
{
private:
    AbstractBoard * const board;
private:
    bool commited;
    QStringList mfileNames;
public:
    explicit FileTransaction(AbstractBoard *b) :
        board(b)
    {
        commited = false;
    }
    ~FileTransaction()
    {
        if (commited)
            return;
        if (!board)
            return;
        board->deleteFiles(mfileNames);
    }
public:
    void commit()
    {
        commited = true;
    }
    QStringList fileNames() const
    {
        return mfileNames;
    }
    bool saveFile(const Tools::File &f)
    {
        if (!board)
            return false;
        bool b = false;
        QString fn = board->saveFile(f, &b);
        if (!b)
            return false;
        mfileNames << fn;
        return true;
    }
    bool saveFileHash(const QString &hashString, QString *error, QString *description, const QLocale &l)
    {
        TranslatorQt tq(l);
        if (!board) {
            return bRet(error, tq.translate("FileTransaction", "Internal error", "error"), description,
                        tq.translate("FileTransaction", "Internal logic error", "description"), false);
        }
        bool ok = false;
        QByteArray fh = Tools::toHashpass(hashString, &ok);
        if (!ok || fh.isEmpty()) {
            return bRet(error, tq.translate("FileTransaction", "Invalid file hash", "error"), description,
                        tq.translate("FileTransaction", "Invalid file hash provided", "description"), false);
        }
        QStringList paths = Database::fileHashPaths(fh, &ok);
        if (!ok) {
            return bRet(error, tq.translate("FileTransaction", "Internal error", "error"), description,
                        tq.translate("FileTransaction", "Internal database error", "description"), false);
        }
        if (paths.isEmpty()) {
            return bRet(error, tq.translate("FileTransaction", "No source file", "error"), description,
                        tq.translate("FileTransaction", "No source file for this file hash", "description"), false);
        }
        QString storagePath = Tools::storagePath();
        if (storagePath.isEmpty()) {
            return bRet(error, tq.translate("FileTransaction", "Internal error", "error"), description,
                        tq.translate("FileTransaction", "Internal file system error", "description"), false);
        }
        QString sfn = storagePath + "/img/" + paths.first();
        QFileInfo fi(sfn);
        QString path = fi.path();
        QString baseName = fi.baseName();
        QString suffix = fi.suffix();
        QStringList sl = BDirTools::entryList(path, QStringList() << (baseName + "s.*"), QDir::Files);
        QString sofn = path + "/" + baseName + ".ololord-file-info";
        if (!fi.exists() || sl.size() != 1 || !QFileInfo(sofn).exists()) {
            return bRet(error, tq.translate("FileTransaction", "Internal error", "error"), description,
                        tq.translate("FileTransaction", "Internal file system error", "description"), false);
        }
        QString spfn = sl.first();
        QString dt = QString::number(QDateTime::currentDateTimeUtc().toMSecsSinceEpoch());
        path = QFileInfo(path).path() + "/" + board->name();
        QString fn = path + "/" + dt + "." + suffix;
        QString pfn = path + "/" + dt + "s." + QFileInfo(spfn).suffix();
        QString ofn = path + "/" + dt + ".ololord-file-info";
        QVariantMap m = BeQt::deserialize(BDirTools::readFile(sofn, -1, &ok)).toMap();
        if (!ok) {
            return bRet(error, tq.translate("FileTransaction", "Internal error", "error"), description,
                        tq.translate("FileTransaction", "Internal file system error", "description"), false);
        }
        m["thumbFileName"] = QFileInfo(pfn).fileName();
        if (!Database::addFileHash(hashString, board->name() + "/" + dt + "." + suffix)) {
            return bRet(error, tq.translate("FileTransaction", "Internal error", "error"), description,
                        tq.translate("FileTransaction", "Internal database error", "description"), false);
        }
        if (!QFile::copy(sfn, fn) || !QFile::copy(spfn, pfn) || !BDirTools::writeFile(ofn, BeQt::serialize(m))) {
            QFile::remove(fn);
            QFile::remove(pfn);
            QFile::remove(ofn);
            return bRet(error, tq.translate("FileTransaction", "Internal error", "error"), description,
                        tq.translate("FileTransaction", "Internal file system error", "description"), false);
        }
        mfileNames << QFileInfo(fn).fileName();
        return bRet(error, QString(), description, QString(), true);
    }
};

static bool addToReferencedPosts(const QString &boardName, quint64 postNumber, quint64 threadNumber,
                                 const Post::RefMap &referencedPosts, QString *error = 0, QString *description = 0)
{
    TranslatorQt tq;
    if (!postNumber) {
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
        foreach (const Post::RefKey &key, referencedPosts.keys()) {
            if (!key.isValid()) {
                return bRet(error, tq.translate("addToReferencedPosts", "Internal error", "error"), description,
                            tq.translate("addToReferencedPosts", "Internal logic error", "description"), false);
            }
            Result<Post> post = queryOne<Post, Post>(odb::query<Post>::board == key.boardName
                                                     && odb::query<Post>::number == key.postNumber);
            if (post.error) {
                return bRet(error, tq.translate("addToReferencedPosts", "Internal error", "error"), description,
                            tq.translate("addToReferencedPosts", "Internal database error", "description"), false);
            }
            if (!post.data) {
                return bRet(error, tq.translate("addToReferencedPosts", "No such post", "error"), description,
                            tq.translate("addToReferencedPosts", "There is no such post", "description"), false);
            }
            post->addReferencedBy(Post::RefKey(boardName, postNumber), threadNumber);
            update(post);
            Cache::removePost(key.boardName, key.postNumber);
        }
        t.commit();
        return bRet(error, QString(), description, QString(), true);
    }  catch (const odb::exception &e) {
        return bRet(error, tq.translate("addToReferencedPosts", "Internal error", "error"), description,
                    Tools::fromStd(e.what()), false);
    }
}

static bool removeFromReferencedPosts(quint64 postNumber, const QString &boardName, QString *error = 0)
{
    TranslatorQt tq;
    if (!postNumber || boardName.isEmpty() || !AbstractBoard::boardNames().contains(boardName))
        return bRet(error, tq.translate("removeFromReferencedPosts", "Internal logic error", "description"), false);
    try {
        Transaction t;
        if (!t) {
            return bRet(error, tq.translate("removeFromReferencedPosts", "Internal database error", "description"),
                        false);
        }
        QList<Post> list = queryAll<Post>();
        foreach (Post post, list) {
            if (!post.removeReferencedBy(boardName, postNumber))
                continue;
            t->update(post);
            Cache::removePost(post.board(), post.number());
        }
        t.commit();
        return bRet(error, QString(), true);
    }  catch (const odb::exception &e) {
        return bRet(error, Tools::fromStd(e.what()), false);
    }
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

static bool createPostInternal(const cppcms::http::request &req, const Tools::PostParameters &param,
                               const Tools::FileList &files, unsigned int bumpLimit, unsigned int postLimit,
                               QString *error, const QLocale &l, QString *description, QDateTime dt = QDateTime(),
                               quint64 threadNumber = 0L, quint64 *pn = 0, Post::RefMap *referencedPosts = 0)
{
    QString boardName = param.value("board");
    AbstractBoard *board = AbstractBoard::board(boardName);
    TranslatorQt tq(l);
    bSet(pn, quint64(0L));
    if (!board) {
        return bRet(error, tq.translate("createPostInternalt", "Internal error", "error"), description,
                           tq.translate("createPostInternalt", "Internal logic error", "description"), false);
    }
    bool isThread = threadNumber;
    if (!threadNumber)
        threadNumber = param.value("thread").toULongLong();
    Tools::Post post = Tools::toPost(param, files);
    try {
        Transaction t;
        if (!t) {
            return bRet(error, tq.translate("createPostInternalt", "Internal error", "error"), description,
                        tq.translate("createPostInternalt", "Internal database error", "description"), false);
        }
        QString err;
        quint64 postNumber = dt.isValid() ? lastPostNumber(boardName, &err, tq.locale())
                                          : incrementPostCounter(boardName, &err, tq.locale());
        if (!postNumber) {
            return bRet(error, tq.translate("createPostInternalt", "Internal error", "error"), description, err,
                        false);
        }
        Result<Thread> thread = queryOne<Thread, Thread>(odb::query<Thread>::number == threadNumber
                                                         && odb::query<Thread>::board == boardName);
        if (thread.error) {
            return bRet(error, tq.translate("createPostInternalt", "Internal error", "error"), description,
                        tq.translate("createPostInternalt", "Internal database error", "description"), false);
        }
        if (!thread) {
            return bRet(error, tq.translate("createPostInternalt", "No such thread", "error"), description,
                        tq.translate("createPostInternalt", "There is no such thread", "description"), false);
        }
        QString ip = Tools::userIp(req);
        if (Tools::captchaEnabled(boardName)) {
            if (board->captchaQuota(ip)) {
                board->captchaUsed(ip);
            } else {
                if (!board->isCaptchaValid(req, param, err)) {
                    return bRet(error, tq.translate("createPostInternalt", "Invalid captcha", "error"), description,
                                err, false);
                }
                board->captchaSolved(ip);
            }
        }
        if (dt.isValid() && post.files.isEmpty() && post.fileHashes.isEmpty()) {
            return bRet(error, tq.translate("createPostInternalt", "No file", "error"), description,
                        tq.translate("createPostInternalt", "Attempt to create a thread without attaching a file",
                                     "description"), false);
        }
        if (post.text.isEmpty() && post.files.isEmpty() && post.fileHashes.isEmpty()) {
            return bRet(error, tq.translate("createPostInternalt", "No file/text", "error"), description,
                        tq.translate("createPostInternalt", "Both file and comment are missing", "description"),
                        false);
        }
        bool bump = post.email.compare("sage", Qt::CaseInsensitive);
        if (postLimit || bumpLimit) {
            Result<PostCount> postCount = queryOne<PostCount, Post>(odb::query<Post>::thread == thread->id());
            if (postCount.error || !postCount) {
                return bRet(error, tq.translate("createPostInternalt", "Internal error", "error"), description,
                            tq.translate("createPostInternalt", "Internal database error", "description"), false);
            }
            if (postLimit && (postCount->count >= (int) postLimit)) {
                return bRet(error, tq.translate("createPostInternalt", "Post limit", "error"), description,
                            tq.translate("createPostInternalt", "The thread has reached it's post limit",
                                         "description"), false);
            }
            if (bumpLimit && (postCount->count >= (int) bumpLimit))
                bump = false;
        }
        if (!dt.isValid())
            dt = QDateTime::currentDateTimeUtc();
        QByteArray hp = Tools::hashpass(req);
        QSharedPointer<Post> p(new Post(boardName, postNumber, dt, thread.data, ip, post.password, hp));
        p->setEmail(post.email);
        FileTransaction ft(board);
        foreach (const Tools::File &f, post.files) {
            if (!ft.saveFile(f)) {
                return bRet(error, tq.translate("createPostInternalt", "Internal error", "error"), description,
                            tq.translate("createPostInternalt", "Internal file system error", "description"), false);
            }
        }
        foreach (const QString &fhs, post.fileHashes) {
            if (!ft.saveFileHash(fhs, error, description, tq.locale()))
                return false;
        }
        p->setFiles(ft.fileNames());
        p->setName(post.name);
        p->setSubject(post.subject);
        p->setRawText(post.text);
        if (board->draftsEnabled() && post.draft)
            p->setDraft(true);
        if (post.raw && registeredUserLevel(req) >= RegisteredUser::AdminLevel) {
            p->setText(post.text);
            p->setRawHtml(true);
        } else {
            Post::RefMap refs;
            p->setText(Controller::processPostText(post.text, boardName, !p->draft() ? &refs : 0));
            if (!p->draft()) {
                if (!addToReferencedPosts(boardName, postNumber, threadNumber, refs, error, description))
                    return false;
                bSet(referencedPosts, refs);
            }
        }
        p->setShowTripcode(!Tools::cookieValue(req, "show_tripcode").compare("true", Qt::CaseInsensitive));
        if (bump) {
            thread->setDateTime(dt);
            update(thread);
        }
        board->beforeStoring(p.data(), param, isThread);
        t->persist(p);
        bSet(pn, postNumber);
        ft.commit();
        return bRet(error, QString(), description, QString(), true);
    } catch (const odb::exception &e) {
        return bRet(error, tq.translate("createPostInternalt", "Internal error", "error"), description,
                    Tools::fromStd(e.what()), false);
    }
}

static bool deletePostInternal(const QString &boardName, quint64 postNumber, QString *error, const QLocale &l)
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
        if (thread) {
            QList<Post> posts = query<Post, Post>(odb::query<Post>::thread == thread->id());
            QStringList files;
            foreach (const Post &p, posts)
                files += p.files();
            t->erase_query<Post>(odb::query<Post>::thread == thread->id());
            t->erase_query<Thread>(odb::query<Thread>::id == thread->id());
            board->deleteFiles(files);
        } else {
            QStringList files = post->files();
            t->erase_query<Post>(odb::query<Post>::board == boardName && odb::query<Post>::number == postNumber);
            board->deleteFiles(files);
        }
        foreach (const Post::RefKey &key, post->referencedBy().keys()) {
            Result<Post> p = queryOne<Post, Post>(odb::query<Post>::board == key.boardName
                                                  && odb::query<Post>::number == key.postNumber
                                                  && odb::query<Post>::rawHtml == false);
            if (p.error)
                return bRet(error, tq.translate("deletePostInternal", "Internal database error", "error"), false);
            if (!p)
                return bRet(error, tq.translate("deletePostInternal", "No such post", "error"), false);
            p->setText(Controller::processPostText(p->rawText(), key.boardName));
            Cache::removePost(key.boardName, key.postNumber);
            update(p);
        }
        if (!removeFromReferencedPosts(postNumber, boardName, error))
            return false;
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

bool addFileHash(const QByteArray &data, const QString &path)
{
    if (data.isEmpty() || path.isEmpty())
        return false;
    QByteArray hash = QCryptographicHash::hash(data, QCryptographicHash::Sha1);
    try {
        Transaction t;
        if (!t)
            return false;
        FileHash fh(path, hash);
        t->persist(fh);
        t.commit();
        return true;
    } catch (const odb::exception &e) {
        qDebug() << Tools::fromStd(e.what());
        return false;
    }
}

bool addFileHash(const QString &hashString, const QString &path)
{
    if (hashString.isEmpty())
        return false;
    bool ok = false;
    QByteArray hash = Tools::toHashpass(hashString, &ok);
    if (!ok)
        return false;
    try {
        Transaction t;
        if (!t)
            return false;
        FileHash fh(path, hash);
        t->persist(fh);
        t.commit();
        return true;
    } catch (const odb::exception &e) {
        qDebug() << Tools::fromStd(e.what());
        return false;
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
    }  catch (const odb::exception &e) {
        qDebug() << e.what();
    }
}

bool createPost(CreatePostParameters &p, quint64 *postNumber)
{
    bSet(postNumber, quint64(0L));
    TranslatorQt tq(p.locale);
    try {
        Transaction t;
        QString err;
        QString desc;
        if (!createPostInternal(p.request, p.params, p.files, p.bumpLimit, p.postLimit, &err, p.locale, &desc,
                                QDateTime(), 0L, postNumber, &p.referencedPosts))
            return bRet(p.error, err, p.description, desc, false);
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
    QString boardName = p.params.value("board");
    AbstractBoard *board = AbstractBoard::board(boardName);
    TranslatorQt tq(p.locale);
    if (!board) {
        return bRet(p.error, tq.translate("createThread", "Internal error", "error"), p.description,
                    tq.translate("createThread", "Internal logic error", "description"), 0L);
    }
    try {
        Transaction t;
        QString err;
        QString desc;
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
                        if (!deletePostInternal(boardName, archivedList.last().number, p.description, p.locale))
                            return bRet(p.error, tq.translate("createThread", "Internal error", "error"), 0L);
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
                    if (!deletePostInternal(boardName, list.last().number, p.description, p.locale))
                        return bRet(p.error, tq.translate("createThread", "Internal error", "error"), 0L);
                }
            }
        }
        QDateTime dt = QDateTime::currentDateTimeUtc();
        QSharedPointer<Thread> thread(new Thread(p.params.value("board"), postNumber, dt));
        if (board->draftsEnabled() && p.params.value("draft").compare("true", Qt::CaseInsensitive))
            thread->setDraft(true);
        t->persist(thread);
        if (!createPostInternal(p.request, p.params, p.files, 0, 0, &err, p.locale, &desc, dt, postNumber))
            return bRet(p.error, err, p.description, desc, 0L);
        t.commit();
        return bRet(p.error, QString(), p.description, QString(), postNumber);
    } catch (const odb::exception &e) {
        return bRet(p.error, tq.translate("createThread", "Internal error", "error"), p.description,
                    Tools::fromStd(e.what()), 0L);
    }
}

bool deletePost(const QString &boardName, quint64 postNumber, QString *error, const QLocale &l)
{
    return deletePostInternal(boardName, postNumber, error, l);
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
        if (!deletePostInternal(boardName, postNumber, error, tq.locale()))
            return false;
        t.commit();
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
        if (p.text.isEmpty() && post->files().isEmpty())
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
        post->setDraft(board->draftsEnabled() && p.draft);
        if (!post->draft() && !removeFromReferencedPosts(p.postNumber, p.boardName, p.error))
            return false;
        if (p.raw && lvl >= RegisteredUser::AdminLevel) {
            post->setText(p.text);
            post->setRawHtml(true);
        } else {
            post->setRawHtml(false);
            post->setText(Controller::processPostText(p.text, p.boardName, !post->draft() ? &p.referencedPosts : 0));
            if (!post->draft() && !addToReferencedPosts(p.boardName, p.postNumber, post->thread().load()->number(),
                                                        p.referencedPosts, p.error)) {
                return false;
            }
        }
        post->setEmail(p.email);
        post->setName(p.name);
        post->setSubject(p.subject);
        bool draftLast = post->draft();
        if (post->draft() != draftLast) {
            Thread thread = *post->thread().load();
            if (thread.number() == post->number()) {
                thread.setDraft(post->draft());
                t->update(thread);
            }
        }
        update(post);
        Cache::removePost(p.boardName, p.postNumber);
        t.commit();
        return bRet(p.error, QString(), true);
    } catch (const odb::exception &e) {
        return bRet(p.error, Tools::fromStd(e.what()), false);
    }
}

bool fileHashExists(const QByteArray &hash, bool *ok)
{
    if (hash.isEmpty())
        return bRet(ok, false, false);
    try {
        Transaction t;
        if (!t)
            return bRet(ok, false, false);
        Result<FileHashCount> count = queryOne<FileHashCount, FileHash>(odb::query<FileHash>::hash == hash);
        if (count.error)
            return bRet(ok, false, false);
        t.commit();
        return bRet(ok, true, count->count > 0);
    } catch (const odb::exception &e) {
        qDebug() << Tools::fromStd(e.what());
        return bRet(ok, false, false);
    }
}

bool fileHashExists(const QString &hashString, bool *ok)
{
    if (hashString.isEmpty())
        return bRet(ok, false, false);
    bool b = false;
    QByteArray hash = Tools::toHashpass(hashString, &b);
    if (!b || hash.isEmpty())
        return bRet(ok, false, false);
    return fileHashExists(hash, ok);
}

QStringList fileHashPaths(const QByteArray &hash, bool *ok)
{
    if (hash.isEmpty())
        return bRet(ok, false, QStringList());
    try {
        Transaction t;
        if (!t)
            return bRet(ok, false, QStringList());
        QList<FileHash> list = query<FileHash, FileHash>(odb::query<FileHash>::hash == hash);
        t.commit();
        QStringList sl;
        foreach (const FileHash &fh, list)
            sl << fh.path();
        return bRet(ok, true, sl);
    } catch (const odb::exception &e) {
        qDebug() << Tools::fromStd(e.what());
        return bRet(ok, false, QStringList());
    }
}

QStringList fileHashPaths(const QString &hashString, bool *ok)
{
    if (hashString.isEmpty())
        return bRet(ok, false, QStringList());
    bool b = false;
    QByteArray hash = Tools::toHashpass(hashString, &b);
    if (!b || hash.isEmpty())
        return bRet(ok, false, QStringList());
    return fileHashPaths(hash, ok);
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

bool removeFileHash(const QString &path)
{
    if (path.isEmpty())
        return false;
    try {
        Transaction t;
        if (!t)
            return false;
        Result<FileHash> hash = queryOne<FileHash, FileHash>(odb::query<FileHash>::path == path);
        if (hash.error)
            return false;
        if (!hash)
            return true;
        erase(hash);
        t.commit();
        return true;
    } catch (const odb::exception &e) {
        qDebug() << Tools::fromStd(e.what());
        return false;
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
