#include "database.h"

#include "board/abstractboard.h"
#include "stored/banneduser.h"
#include "stored/banneduser-odb.hxx"
#include "stored/postcounter.h"
#include "stored/postcounter-odb.hxx"
#include "stored/registereduser.h"
#include "stored/registereduser-odb.hxx"
#include "stored/thread.h"
#include "stored/thread-odb.hxx"
#include "tools.h"
#include "translator.h"

#include <BDirTools>

#include <QCryptographicHash>
#include <QDebug>
#include <QScopedPointer>
#include <QSharedPointer>
#include <QString>
#include <QStringList>

#include <cppcms/http_cookie.h>
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
    const QString BoardName;
private:
    bool commited;
    QStringList mfileNames;
public:
    explicit FileTransaction(const QString &boardName) :
        BoardName(boardName)
    {
        commited = false;
    }
    ~FileTransaction()
    {
        if (commited)
            return;
        Tools::deleteFiles(BoardName, mfileNames);
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
    QString saveFile(const Tools::File &f, bool *ok = 0)
    {
        bool b = false;
        QString fn = Tools::saveFile(f, BoardName, &b);
        if (b)
            mfileNames << fn;
        return bRet(ok, b, fn);
    }
};

static bool banUserInternal(const QString &sourceBoard, quint64 postNumber, const QString &board, int level,
                            const QString &reason, const QDateTime &expires, QString *error, const QLocale &l,
                            QString ip = QString())
{
    TranslatorQt tq(l);
    if (board.isEmpty() || (!AbstractBoard::boardNames().contains(board) && "*" != board))
        return bRet(error, tq.translate("banUserInternal", "Invalid board name", "error"), false);
    try {
        Transaction t;
        if (!t.db())
            return bRet(error, tq.translate("banUserInternal", "Internal database error", "error"), false);
        quint64 postId = 0L;
        if (!sourceBoard.isEmpty() && postNumber) {
            Result<Post> post = queryOne<Post, Post>(t.db(), odb::query<Post>::board == sourceBoard
                                                     && odb::query<Post>::number == postNumber);
            if (post.error)
                return bRet(error, tq.translate("banUserInternal", "Internal database error", "error"), false);
            if (!post.data)
                return bRet(error, tq.translate("banUserInternal", "No such post", "error"), false);
            postId = post->id();
            if (ip.isEmpty())
                ip = post->posterIp();
            post->setBannedFor(level > 0);
            update(t.db(), post);
        }
        if (ip.isEmpty())
            return bRet(error, tq.translate("banUserInternal", "Invalid IP address", "error"), false);
        Result<BannedUser> user = queryOne<BannedUser, BannedUser>(t.db(), odb::query<BannedUser>::board == board
                                                                   && odb::query<BannedUser>::ip == ip);
        if (user.error)
            return bRet(error, tq.translate("banUserInternal", "Internal database error", "error"), false);
        QDateTime dt = QDateTime::currentDateTimeUtc();
        if (!user) {
            if (level < 1) {
                t.commit();
                return bRet(error, QString(), true);
            }
            user = new BannedUser(board, ip, dt, postId);
            persist(t.db(), user);
        } else {
            if (level < 1) {
                t.db()->erase(*user);
                t.commit();
                return bRet(error, QString(), true);
            }
            user->setDateTime(dt);
            user->setExpirationDateTime(expires);
            user->setLevel(level);
            user->setReason(reason);
            update(t.db(), user);
        }
        t.commit();
        return bRet(error, QString(), true);
    }  catch (const odb::exception &e) {
        return bRet(error, Tools::fromStd(e.what()), false);
    }
}

static bool createPostInternal(odb::database *db, const cppcms::http::request &req, const Tools::PostParameters &param,
                               const Tools::FileList &files, unsigned int bumpLimit, unsigned int postLimit,
                               QString *error, const QLocale &l, QString *description, QDateTime dt = QDateTime(),
                               quint64 threadNumber = 0L)
{
    QString boardName = param.value("board");
    if (!threadNumber)
        threadNumber = param.value("thread").toULongLong();
    Tools::Post post = Tools::toPost(param, files);
    TranslatorQt tq(l);
    try {
        if (!db) {
            return bRet(error, tq.translate("createPostInternalt", "Internal error", "error"), description,
                        tq.translate("createPostInternalt", "Internal database error", "description"), false);
        }
        QString err;
        quint64 postNumber = dt.isValid() ? lastPostNumber(db, boardName, &err, tq.locale())
                                          : incrementPostCounter(db, boardName, &err, tq.locale());
        if (!postNumber) {
            return bRet(error, tq.translate("createPostInternalt", "Internal error", "error"), description, err,
                        false);
        }
        Result<Thread> thread = queryOne<Thread, Thread>(db, odb::query<Thread>::number == threadNumber
                                                         && odb::query<Thread>::board == boardName);
        if (thread.error) {
            return bRet(error, tq.translate("createPostInternalt", "Internal error", "error"), description,
                        tq.translate("createPostInternalt", "Internal database error", "description"), false);
        }
        if (!thread) {
            return bRet(error, tq.translate("createPostInternalt", "No such thread", "error"), description,
                        tq.translate("createPostInternalt", "There is no such thread", "description"), false);
        }
        if (Tools::captchaEnabled(boardName) && !Tools::isCaptchaValid(post.captcha)) {
            return bRet(error, tq.translate("createPostInternalt", "Invalid captcha", "error"), description,
                        tq.translate("createPostInternalt", "Captcha is missing or invalid", "description"), false);
        }
        if (dt.isValid() && post.files.isEmpty()) {
            return bRet(error, tq.translate("createPostInternalt", "No file", "error"), description,
                        tq.translate("createPostInternalt", "Attempt to create a thread without attaching a file",
                                     "description"), false);
        }
        if (post.text.isEmpty() && post.files.isEmpty()) {
            return bRet(error, tq.translate("createPostInternalt", "No file/text", "error"), description,
                        tq.translate("createPostInternalt", "Both file and comment are missing", "description"),
                        false);
        }
        bool bump = post.email.compare("sage", Qt::CaseInsensitive);
        if (postLimit || bumpLimit) {
            Result<PostCount> postCount = queryOne<PostCount, Post>(db, odb::query<Post>::thread == thread->id());
            if (postCount.error || !postCount) {
                return bRet(error, tq.translate("createPostInternalt", "Internal error", "error"), description,
                            tq.translate("createPostInternalt", "Internal database error", "description"), false);
            }
            if (postLimit && (postCount->count >= (int) postLimit)) {
                return bRet(error, tq.translate("createPostInternalt", "Post limit", "error"), description,
                            tq.translate("createPostInternalt", "The thread has reached it's post limit", "description"),
                            false);
            }
            if (bumpLimit && (postCount->count >= (int) bumpLimit))
                bump = false;
        }
        if (!dt.isValid())
            dt = QDateTime::currentDateTimeUtc();
        QByteArray hp = Tools::hashpass(req);
        QSharedPointer<Post> p(new Post(boardName, postNumber, dt, thread.data, Tools::userIp(req), post.password, hp));
        p->setEmail(post.email);
        FileTransaction ft(boardName);
        foreach (const Tools::File &f, post.files) {
            bool ok = false;
            ft.saveFile(f, &ok);
            if (!ok) {
                return bRet(error, tq.translate("createPostInternalt", "Internal error", "error"), description,
                            tq.translate("createPostInternalt", "Internal file system error", "description"), false);
            }
        }
        p->setFiles(ft.fileNames());
        p->setName(post.name);
        p->setSubject(post.subject);
        p->setText(post.text);
        if (bump) {
            thread->setDateTime(dt);
            update(db, thread);
        }
        db->persist(p);
        ft.commit();
        return bRet(error, QString(), description, QString(), true);
    } catch (const odb::exception &e) {
        return bRet(error, tq.translate("createPostInternalt", "Internal error", "error"), description,
                    Tools::fromStd(e.what()), false);
    }
}

static bool deletePostInternal(const QString &boardName, quint64 postNumber, QString *error, const QLocale &l,
                               odb::database *db = 0)
{
    TranslatorQt tq(l);
    if (boardName.isEmpty() || !AbstractBoard::boardNames().contains(boardName))
        return bRet(error, tq.translate("deletePostInternal", "Invalid board name", "error"), false);
    if (!postNumber)
        return bRet(error, tq.translate("deletePostInternal", "Invalid post number", "error"), false);
    try {
        Transaction t(db);
        if (!t.db())
            return bRet(error, tq.translate("deletePostInternal", "Internal database error", "error"), false);
        Result<Thread> thread = queryOne<Thread, Thread>(t.db(), odb::query<Thread>::board == boardName
                                                         && odb::query<Thread>::number == postNumber);
        if (thread.error)
            return bRet(error, tq.translate("deletePostInternal", "Internal database error", "error"), false);
        if (thread) {
            QList<Post> posts = query<Post, Post>(t.db(), odb::query<Post>::thread == thread->id());
            QStringList files;
            foreach (const Post &p, posts)
                files += p.files();
            t.db()->erase_query<Post>(odb::query<Post>::thread == thread->id());
            t.db()->erase_query<Thread>(odb::query<Thread>::id == thread->id());
            Tools::deleteFiles(boardName, files);
        } else {
            Result<Post> post = queryOne<Post, Post>(t.db(), odb::query<Post>::board == boardName
                                                     && odb::query<Post>::number == postNumber);
            if (post.error)
                return bRet(error, tq.translate("deletePostInternal", "Internal database error", "error"), false);
            if (!post)
                return bRet(error, tq.translate("deletePostInternal", "No such post", "error"), false);
            QStringList files = post->files();
            t.db()->erase_query<Post>(odb::query<Post>::board == boardName && odb::query<Post>::number == postNumber);
            Tools::deleteFiles(boardName, files);
        }
        t.commit();
        return bRet(error, QString(), true);
    }  catch (const odb::exception &e) {
        return bRet(error, Tools::fromStd(e.what()), false);
    }
}

static bool setThreadFixedInternal(const QString &board, quint64 threadNumber, bool fixed, QString *error,
                                   const QLocale &l, odb::database *db = 0)
{
    TranslatorQt tq(l);
    try {
        Transaction t(db);
        if (!t.db())
            return bRet(error, tq.translate("setThreadFixedInternal", "Internal database error", "error"), false);
        Result<Thread> thread = queryOne<Thread, Thread>(t.db(), odb::query<Thread>::number == threadNumber
                                                         && odb::query<Thread>::board == board);
        if (thread.error)
            return bRet(error, tq.translate("setThreadFixedInternal", "Internal database error", "error"), false);
        if (!thread)
            return bRet(error, tq.translate("setThreadFixedInternal", "No such thread", "error"), false);
        if (thread->fixed() == fixed)
            return bRet(error, QString(), true);
        thread->setFixed(fixed);
        update(t.db(), thread);
        t.commit();
        return bRet(error, QString(), true);
    } catch (const odb::exception &e) {
        return bRet(error, Tools::fromStd(e.what()), false);
    }
}

static bool setThreadOpenedInternal(const QString &board, quint64 threadNumber, bool opened, QString *error,
                                    const QLocale &l, odb::database *db = 0)
{
    TranslatorQt tq(l);
    try {
        Transaction t(db);
        if (!t.db())
            return bRet(error, tq.translate("setThreadOpenedInternal", "Internal database error", "error"), false);
        Result<Thread> thread = queryOne<Thread, Thread>(t.db(), odb::query<Thread>::number == threadNumber
                                                         && odb::query<Thread>::board == board);
        if (thread.error)
            return bRet(error, tq.translate("setThreadOpenedInternal", "Internal database error", "error"), false);
        if (!thread)
            return bRet(error, tq.translate("setThreadOpenedInternal", "No such thread", "error"), false);
        if (thread->postingEnabled() == opened)
            return bRet(error, QString(), true);
        thread->setPostingEnabled(opened);
        update(t.db(), thread);
        t.commit();
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

void checkOutdatedEntries()
{
    try {
        Transaction t;
        if (!t.db())
            return;
        QDateTime dt = QDateTime::currentDateTimeUtc();
        QList<BannedUser> list = queryAll<BannedUser>(t.db());
        foreach (const BannedUser &u, list) {
            QDateTime exp = u.expirationDateTime();
            if (exp.isValid() && exp <= dt) {
                quint64 postId = u.postId();
                t.db()->erase_query<BannedUser>(odb::query<BannedUser>::id == u.id());
                if (postId) {
                    Result<Post> post = queryOne<Post, Post>(t.db(), odb::query<Post>::id == postId);
                    if (post.error || !post)
                        continue;
                    post->setBannedFor(false);
                    persist(t.db(), post);
                }
            }
        }
        t.commit();
    }  catch (const odb::exception &e) {
        qDebug() << e.what();
    }
}

odb::database *createConnection()
{
    QString storagePath = Tools::storagePath();
    if (storagePath.isEmpty())
        return 0;
    QString fileName = storagePath + "/db.sqlite";
    if (!BDirTools::touch(fileName))
        return 0;
    odb::database *db = 0;
    try {
        db = new odb::sqlite::database(Tools::toStd(fileName), SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE);
        if (!db)
            return 0;
        QScopedPointer<odb::transaction> t;
        bool trans = !odb::transaction::has_current();
        if (trans)
            t.reset(new odb::transaction(db->begin()));
        if (!db->execute("SELECT 1 FROM sqlite_master WHERE type='table' AND name='threads'")) {
            if (trans)
                t->commit();
            if (trans)
                t->reset(db->begin());
            db->execute("PRAGMA foreign_keys=OFF");
            odb::schema_catalog::create_schema(*db);
            db->execute("PRAGMA foreign_keys=ON");
            if (trans)
                t->commit();
        }
        return db;
    } catch (const odb::exception &e) {
        qDebug() << e.what();
        delete db;
        return 0;
    }
}

bool createPost(CreatePostParameters &p)
{
    TranslatorQt tq(p.locale);
    try {
        Transaction t;
        QString err;
        QString desc;
        if (!createPostInternal(t.db(), p.request, p.params, p.files, p.bumpLimit, p.postLimit, &err, p.locale, &desc))
            return bRet(p.error, err, p.description, desc, false);
        t.commit();
        return bRet(p.error, QString(), p.description, QString(), true);
    } catch (const odb::exception &e) {
        return bRet(p.error, tq.translate("createPost", "Internal error", "error"), p.description,
                    Tools::fromStd(e.what()), false);
    }
}

quint64 createThread(CreateThreadParameters &p)
{
    QString boardName = p.params.value("board");
    TranslatorQt tq(p.locale);
    try {
        Transaction t;
        QString err;
        QString desc;
        quint64 postNumber = incrementPostCounter(t.db(), p.params.value("board"), &err, p.locale);
        if (!postNumber)
            return bRet(p.error, tq.translate("createThread", "Internal error", "error"), p.description, err, 0L);
        if (p.threadLimit) {
            Result<ThreadCount> threadCount = queryOne<ThreadCount, Thread>(
                        t.db(), odb::query<Thread>::board == boardName && odb::query<Thread>::archived == false);
            if (threadCount.error || !threadCount) {
                return bRet(p.error, tq.translate("createThread", "Internal error", "error"),
                            p.description, tq.translate("createThread", "Internal database error", "error"), 0L);
            }
            if (threadCount->count >= (int) p.threadLimit) {
                QList<ThreadIdDateTimeFixed> list = query<ThreadIdDateTimeFixed, Thread>(
                            t.db(), odb::query<Thread>::board == boardName && odb::query<Thread>::archived == false);
                qSort(list.begin(), list.end(), &threadIdDateTimeFixedLessThan);
                if (p.archiveLimit) {
                    Result<ThreadCount> archivedCount = queryOne<ThreadCount, Thread>(
                                t.db(), odb::query<Thread>::board == boardName
                                && odb::query<Thread>::archived == true);
                    if (archivedCount.error || !archivedCount) {
                        return bRet(p.error, tq.translate("createThread", "Internal error", "error"),
                                    p.description, tq.translate("createThread", "Internal database error", "error"), 0L);
                    }
                    if (archivedCount->count >= (int) p.archiveLimit) {
                        QList<ThreadIdDateTimeFixed> archivedList = query<ThreadIdDateTimeFixed, Thread>(
                                    t.db(), odb::query<Thread>::board == boardName
                                    && odb::query<Thread>::archived == true);
                        qSort(archivedList.begin(), archivedList.end(), &threadIdDateTimeFixedLessThan);
                        if (!deletePostInternal(boardName, archivedList.last().number, p.description, p.locale,
                                                t.db())) {
                            return bRet(p.error, tq.translate("createThread", "Internal error", "error"), 0L);
                        }
                    }
                    Result<Thread> thread = queryOne<Thread, Thread>(t.db(), odb::query<Thread>::id == list.last().id);
                    if (thread.error) {
                        return bRet(p.error, tq.translate("createThread", "Internal error", "error"), p.description,
                                    tq.translate("createThread", "Internal database error", "error"), 0L);
                    }
                    if (!thread) {
                        return bRet(p.error, tq.translate("createThread", "Internal error", "error"), p.description,
                                    tq.translate("createThread", "Internal database error", "error"), 0L);
                    }
                    thread->setArchived(true);
                    update(t.db(), thread);
                } else {
                    if (!deletePostInternal(boardName, list.last().number, p.description, p.locale, t.db()))
                        return bRet(p.error, tq.translate("createThread", "Internal error", "error"), 0L);
                }
            }
        }
        QDateTime dt = QDateTime::currentDateTimeUtc();
        QSharedPointer<Thread> thread(new Thread(p.params.value("board"), postNumber, dt));
        t.db()->persist(thread);
        if (!createPostInternal(t.db(), p.request, p.params, p.files, 0, 0, &err, p.locale, &desc, dt, postNumber))
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
        if (!t.db())
            return bRet(error, tq.translate("deletePost", "Internal database error", "error"), false);
        Result<Post> post = queryOne<Post, Post>(t.db(), odb::query<Post>::board == boardName
                                                 && odb::query<Post>::number == postNumber);
        if (post.error)
            return bRet(error, tq.translate("deletePost", "Internal database error", "error"), false);
        if (!post)
            return bRet(error, tq.translate("deletePost", "No such post", "error"), false);
        if (password.isEmpty()) {
            if (hashpass != post->hashpass()) {
                int lvl = registeredUserLevel(req, t.db());
                if (lvl < RegisteredUser::ModerLevel || registeredUserLevel(post->hashpass(), t.db()) >= lvl)
                    return bRet(error, tq.translate("deletePost", "Not enough rights", "error"), false);
                if (lvl < RegisteredUser::AdminLevel) {
                    QStringList boards = registeredUserBoards(req, t.db());
                    if (!boards.contains("*") && !boards.contains(boardName))
                        return bRet(error, tq.translate("deletePost", "Not enough rights", "error"), false);
                }
            }
        } else if (password != post->password()) {
            return bRet(error, tq.translate("deletePost", "Incorrect password", "error"), false);
        }
        if (!deletePostInternal(boardName, postNumber, error, tq.locale(), t.db()))
            return false;
        t.commit();
        return bRet(error, QString(), true);
    } catch (const odb::exception &e) {
        return bRet(error, Tools::fromStd(e.what()), false);
    }
}

quint64 incrementPostCounter(odb::database *db, const QString &boardName, QString *error, const QLocale &l)
{
    TranslatorQt tq(l);
    if (!db)
        return bRet(error, tq.translate("incrementPostCounter", "Invalid database connection", "error"), 0L);
    if (!AbstractBoard::boardNames().contains(boardName))
        return bRet(error, tq.translate("incrementPostCounter", "Invalid board name", "error"), 0L);
    quint64 incremented = 0L;
    try {
        Result<PostCounter> counter = queryOne<PostCounter, PostCounter>(
                    db, odb::query<PostCounter>::board == boardName);
        if (!counter && !counter.error) {
            PostCounter c(boardName);
            db->persist(c);
            counter = queryOne<PostCounter, PostCounter>(db, odb::query<PostCounter>::board == boardName);
        }
        if (counter.error || !counter)
            return bRet(error, tq.translate("incrementPostCounter", "Internal database error", "error"), 0L);
        incremented = counter->incrementLastPostNumber();
        update(db, counter);
    } catch (const odb::exception &e) {
        return bRet(error, Tools::fromStd(e.what()), 0L);
    }
    return bRet(error, QString(), incremented);
}

quint64 lastPostNumber(odb::database *db, const QString &boardName, QString *error, const QLocale &l)
{
    TranslatorQt tq(l);
    if (!db)
        return bRet(error, tq.translate("lastPostNumber", "Invalid database connection", "error"), 0L);
    if (!AbstractBoard::boardNames().contains(boardName))
        return bRet(error, tq.translate("lastPostNumber", "Invalid board name", "error"), 0L);
    try {
        Result<PostCounter> counter = queryOne<PostCounter, PostCounter>(
                    db, odb::query<PostCounter>::board == boardName);
        if (!counter && !counter.error) {
            PostCounter c(boardName);
            db->persist(c);
            counter = queryOne<PostCounter, PostCounter>(db, odb::query<PostCounter>::board == boardName);
        }
        if (counter.error || !counter)
            return bRet(error, tq.translate("incrementPostCounter", "Internal database error", "error"), 0L);
        return bRet(error, QString(), counter->lastPostNumber());
    } catch (const odb::exception &e) {
        return bRet(error, Tools::fromStd(e.what()), 0L);
    }
}

QString posterIp(const QString &boardName, quint64 postNumber)
{
    try {
        Transaction t;
        if (!t.db())
            return "";
        Result<Post> post = queryOne<Post, Post>(t.db(), odb::query<Post>::board == boardName
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

QStringList registeredUserBoards(const cppcms::http::request &req, odb::database *db)
{
    QByteArray hp = Tools::hashpass(req);
    if (hp.isEmpty())
        return QStringList();
    return registeredUserBoards(hp, db);
}

QStringList registeredUserBoards(const QByteArray &hashpass, odb::database *db)
{
    bool b = false;
    Tools::toString(hashpass, &b);
    if (!b)
        return QStringList();
    try {
        Transaction t(db);
        if (!t.db())
            return QStringList();
        Result<RegisteredUser> user = queryOne<RegisteredUser, RegisteredUser>(
                    t.db(), odb::query<RegisteredUser>::hashpass == hashpass);
        if (user.error || !user)
            return QStringList();
        t.commit();
        return user->boards();
    }  catch (const odb::exception &e) {
        qDebug() << e.what();
        return QStringList();
    }
}

int registeredUserLevel(const cppcms::http::request &req, odb::database *db)
{
    QByteArray hp = Tools::hashpass(req);
    if (hp.isEmpty())
        return -1;
    return registeredUserLevel(hp, db);
}

int registeredUserLevel(const QByteArray &hashpass, odb::database *db)
{
    bool b = false;
    Tools::toString(hashpass, &b);
    if (!b)
        return -1;
    try {
        Transaction t(db);
        if (!t.db())
            return -1;
        Result<RegisteredUserLevel> level = queryOne<RegisteredUserLevel, RegisteredUser>(
                    t.db(), odb::query<RegisteredUser>::hashpass == hashpass);
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
        if (!t.db())
            return bRet(error, tq.translate("registerUser", "Internal database error", "error"), false);
        RegisteredUser user(hashpass, QDateTime::currentDateTimeUtc(), level, boards);
        t.db()->persist(user);
        t.commit();
        return bRet(error, QString(), true);
    } catch (const odb::exception &e) {
        return bRet(error, Tools::fromStd(e.what()), false);
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
        if (!t.db())
            return bRet(error, tq.translate("setThreadFixed", "Internal database error", "error"), false);
        int lvl = registeredUserLevel(req, t.db());
        if (lvl < RegisteredUser::ModerLevel)
            return bRet(error, tq.translate("setThreadFixed", "Not enough rights", "error"), false);
        if (lvl < RegisteredUser::AdminLevel) {
            QStringList boards = registeredUserBoards(req, t.db());
            if (!boards.contains("*") && !boards.contains(boardName))
                return bRet(error, tq.translate("setThreadFixed", "Not enough rights", "error"), false);
        }
        if (!setThreadFixedInternal(boardName, threadNumber, fixed, error, tq.locale(), t.db()))
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
        if (!t.db())
            return bRet(error, tq.translate("setThreadOpened", "Internal database error", "error"), false);
        int lvl = registeredUserLevel(req, t.db());
        if (lvl < RegisteredUser::ModerLevel)
            return bRet(error, tq.translate("setThreadOpened", "Not enough rights", "error"), false);
        if (lvl < RegisteredUser::AdminLevel) {
            QStringList boards = registeredUserBoards(req, t.db());
            if (!boards.contains("*") && !boards.contains(boardName))
                return bRet(error, tq.translate("setThreadOpened", "Not enough rights", "error"), false);
        }
        if (!setThreadOpenedInternal(boardName, threadNumber, opened, error, tq.locale(), t.db()))
            return false;
        t.commit();
        return bRet(error, QString(), true);
    } catch (const odb::exception &e) {
        return bRet(error, Tools::fromStd(e.what()), false);
    }
}

}
