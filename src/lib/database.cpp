#include "database.h"

#include "board/abstractboard.h"
#include "cache.h"
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

#include <QCryptographicHash>
#include <QDebug>
#include <QScopedPointer>
#include <QSharedPointer>
#include <QString>
#include <QStringList>

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
    QString saveFile(const Tools::File &f, bool *ok = 0)
    {
        if (!board)
            return bRet(ok, false, QString());
        bool b = false;
        QString fn = board->saveFile(f, &b);
        if (!b)
            return bRet(ok, false, QString());
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
                return bRet(error, QString(), true);
            }
            user->setDateTime(dt);
            user->setExpirationDateTime(expires);
            user->setLevel(level);
            user->setReason(reason);
            update(user);
        }
        t.commit();
        return bRet(error, QString(), true);
    }  catch (const odb::exception &e) {
        return bRet(error, Tools::fromStd(e.what()), false);
    }
}

static bool createPostInternal(const cppcms::http::request &req, const Tools::PostParameters &param,
                               const Tools::FileList &files, unsigned int bumpLimit, unsigned int postLimit,
                               QString *error, const QLocale &l, QString *description, QDateTime dt = QDateTime(),
                               quint64 threadNumber = 0L)
{
    QString boardName = param.value("board");
    AbstractBoard *board = AbstractBoard::board(boardName);
    TranslatorQt tq(l);
    if (!board) {
        return bRet(error, tq.translate("createPostInternalt", "Internal error", "error"), description,
                           tq.translate("createPostInternalt", "Internal logic error", "description"), false);
    }
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
        p->setShowTripcode(!Tools::cookieValue(req, "show_tripcode").compare("true", Qt::CaseInsensitive));
        if (bump) {
            thread->setDateTime(dt);
            update(thread);
        }
        t->persist(p);
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
            Cache::removeFileInfos(boardName, files);
        } else {
            Result<Post> post = queryOne<Post, Post>(odb::query<Post>::board == boardName
                                                     && odb::query<Post>::number == postNumber);
            if (post.error)
                return bRet(error, tq.translate("deletePostInternal", "Internal database error", "error"), false);
            if (!post)
                return bRet(error, tq.translate("deletePostInternal", "No such post", "error"), false);
            QStringList files = post->files();
            t->erase_query<Post>(odb::query<Post>::board == boardName && odb::query<Post>::number == postNumber);
            board->deleteFiles(files);
            Cache::removeFileInfos(boardName, files);
        }
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
        if (lvl < RegisteredUser::ModerLevel || registeredUserLevel(post->hashpass()) >= lvl)
            return bRet(error, tq.translate("banUser", "Not enough rights", "error"), false);
        if (lvl < RegisteredUser::AdminLevel) {
            QStringList boards = registeredUserBoards(req);
            if (!boards.contains("*") && (!boards.contains(sourceBoard) || !boards.contains(board)))
                return bRet(error, tq.translate("banUser", "Not enough rights", "error"), false);
        }
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
                    persist(post);
                }
            }
        }
        t.commit();
    }  catch (const odb::exception &e) {
        qDebug() << e.what();
    }
}

bool createPost(CreatePostParameters &p)
{
    TranslatorQt tq(p.locale);
    try {
        Transaction t;
        QString err;
        QString desc;
        if (!createPostInternal(p.request, p.params, p.files, p.bumpLimit, p.postLimit, &err, p.locale, &desc))
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
    Transaction t;
    if (!t)
        return;
    if (t->execute("SELECT 1 FROM sqlite_master WHERE type='table' AND name='threads'"))
        return;
    t->execute("PRAGMA foreign_keys=OFF");
    odb::schema_catalog::create_schema(*t);
    t->execute("PRAGMA foreign_keys=ON");
    t.commit();
}

quint64 createThread(CreateThreadParameters &p)
{
    QString boardName = p.params.value("board");
    TranslatorQt tq(p.locale);
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
                        return bRet(p.error, tq.translate("createThread", "Internal error", "error"),
                                    p.description, tq.translate("createThread", "Internal database error", "error"), 0L);
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
                if (lvl < RegisteredUser::ModerLevel || registeredUserLevel(post->hashpass()) >= lvl)
                    return bRet(error, tq.translate("deletePost", "Not enough rights", "error"), false);
                if (lvl < RegisteredUser::AdminLevel) {
                    QStringList boards = registeredUserBoards(req);
                    if (!boards.contains("*") && !boards.contains(boardName))
                        return bRet(error, tq.translate("deletePost", "Not enough rights", "error"), false);
                }
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

bool editPost(const cppcms::http::request &req, const QString &boardName, quint64 postNumber, const QString &text,
              QString *error)
{
    AbstractBoard *board = AbstractBoard::board(boardName);
    TranslatorQt tq(req);
    if (!board)
        return bRet(error, tq.translate("editPost", "Invalid board name", "error"), false);
    if (!postNumber)
        return bRet(error, tq.translate("editPost", "Invalid post number", "error"), false);
    try {
        Transaction t;
        if (!t)
            return bRet(error, tq.translate("editPost", "Internal database error", "error"), false);
        QByteArray hashpass = Tools::hashpass(req);
        if (hashpass.isEmpty())
            return bRet(error, tq.translate("editPost", "Not logged in", "error"), false);
        int lvl = registeredUserLevel(hashpass);
        if (lvl < RegisteredUser::ModerLevel)
            return bRet(error, tq.translate("editPost", "Not enough rights", "error"), false);
        Result<Post> post = queryOne<Post, Post>(odb::query<Post>::number == postNumber
                                                 && odb::query<Post>::board == boardName);
        if (post.error)
            return bRet(error, tq.translate("editPost", "Internal database error", "error"), false);
        if (!post)
            return bRet(error, tq.translate("editPost", "No such post", "error"), false);
        if (post->hashpass() != hashpass && lvl <= registeredUserLevel(post->hashpass()))
            return bRet(error, tq.translate("editPost", "Not enough rights", "error"), false);
        if (text.isEmpty() && post->files().isEmpty())
            return bRet(error, tq.translate("editPost", "No text provided", "error"), false);
        if (text.length() > int(Tools::maxInfo(Tools::MaxTextFieldLength, boardName)))
            return bRet(error, tq.translate("editPost", "Text is too long", "error"), false);
        post->setText(text);
        update(post);
        t.commit();
        return bRet(error, QString(), true);
    } catch (const odb::exception &e) {
        return bRet(error, Tools::fromStd(e.what()), false);
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
        Database::Result<Post> post = Database::queryOne<Post, Post>(odb::query<Post>::board == boardName
                                                                     && odb::query<Post>::number == postNumber);
        if (post.error)
            return bRet(ok, false, error, tq.translate("getPost", "Internal database error", "error"), Post());
        if (!post)
            return bRet(ok, false, error, tq.translate("getPost", "No such post", "error"), Post());
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
        int lvl = registeredUserLevel(req);
        if (lvl < RegisteredUser::ModerLevel)
            return bRet(error, tq.translate("setThreadFixed", "Not enough rights", "error"), false);
        if (lvl < RegisteredUser::AdminLevel) {
            QStringList boards = registeredUserBoards(req);
            if (!boards.contains("*") && !boards.contains(boardName))
                return bRet(error, tq.translate("setThreadFixed", "Not enough rights", "error"), false);
        }
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
        int lvl = registeredUserLevel(req);
        if (lvl < RegisteredUser::ModerLevel)
            return bRet(error, tq.translate("setThreadOpened", "Not enough rights", "error"), false);
        if (lvl < RegisteredUser::AdminLevel) {
            QStringList boards = registeredUserBoards(req);
            if (!boards.contains("*") && !boards.contains(boardName))
                return bRet(error, tq.translate("setThreadOpened", "Not enough rights", "error"), false);
        }
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
        QList<BannedUser> list = Database::query<BannedUser, BannedUser>(odb::query<BannedUser>::board == "*"
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
            list = Database::query<BannedUser, BannedUser>(odb::query<BannedUser>::board == boardName
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
