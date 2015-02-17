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

static bool banUserInternal(const QString &sourceBoard, quint64 postNumber, const QString &board, int level,
                            const QString &reason, const QDateTime &expires, QString *error, const QLocale &l,
                            QString ip = QString())
{
    TranslatorQt tq(l);
    if (board.isEmpty() || (!AbstractBoard::boardNames().contains(board) && "*" != board))
        return bRet(error, tq.translate("banUserInternal", "Invalid board name", "error"), false);
    try {
        QScopedPointer<odb::database> db(createConnection());
        if (!db)
            return bRet(error, tq.translate("banUserInternal", "Internal database error", "error"), false);
        odb::transaction transaction(db->begin());
        if (!sourceBoard.isEmpty() && postNumber) {
            odb::result<Post> r(db->query<Post>(odb::query<Post>::board == sourceBoard
                                                && odb::query<Post>::number == postNumber));
            odb::result<Post>::iterator i = r.begin();
            if (r.end() == i)
                return bRet(error, tq.translate("banUserInternal", "No such post", "error"), false);
            Post post = *i;
            ++i;
            if (r.end() != i)
                return bRet(error, tq.translate("banUserInternal", "Internal database error", "error"), false);
            if (ip.isEmpty())
                ip = post.posterIp();
            post.setBannedFor(level > 0);
            db->update(post);
        }
        if (ip.isEmpty())
            return bRet(error, tq.translate("banUserInternal", "Invalid IP address", "error"), false);
        odb::result<BannedUser> r(db->query<BannedUser>(odb::query<BannedUser>::board == board
                                                        && odb::query<BannedUser>::ip == ip));
        odb::result<BannedUser>::iterator i = r.begin();
        QSharedPointer<BannedUser> user;
        bool upd = false;
        QDateTime dt = QDateTime::currentDateTimeUtc();
        if (r.end() == i) {
            if (level < 1) {
                transaction.commit();
                return bRet(error, QString(), true);
            }
            user = QSharedPointer<BannedUser>(new BannedUser(board, ip, dt));
        } else {
            user = QSharedPointer<BannedUser>(new BannedUser(*i));
            ++i;
            if (r.end() != i)
                return bRet(error, tq.translate("banUserInternal", "Internal database error", "error"), false);
            if (level < 1) {
                db->erase(user);
                transaction.commit();
                return bRet(error, QString(), true);
            }
            user->setDateTime(dt);
            upd = true;
        }
        if (!user)
            return bRet(error, tq.translate("banUserInternal", "Internal error", "error"), false);
        user->setExpirationDateTime(expires);
        user->setLevel(level);
        user->setReason(reason);
        if (upd)
            db->update(*user);
        else
            db->persist(*user);
        transaction.commit();
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
            return bRet(error, tq.translate("createPost", "Internal error", "error"), description,
                        tq.translate("createPost", "Internal database error", "description"), false);
        }
        QString err;
        quint64 postNumber = dt.isValid() ? lastPostNumber(db, boardName, &err, tq.locale())
                                          : incrementPostCounter(db, boardName, &err, tq.locale());
        if (!postNumber)
            return bRet(error, tq.translate("createPost", "Internal error", "error"), description, err, false);
        odb::result<Thread> tr = db->query<Thread>(odb::query<Thread>::number == threadNumber
                                                   && odb::query<Thread>::board == boardName);
        odb::result<Thread>::iterator ti = tr.begin();
        if (tr.end() == ti) {
            return bRet(error, tq.translate("createPost", "No such thread", "error"), description,
                        tq.translate("createPost", "There is no such thread", "description"), false);
        }
        QSharedPointer<Thread> thread(new Thread(*ti));
        ++ti;
        if (tr.end() != ti) {
            return bRet(error, tq.translate("createPost", "Internal error", "error"), description,
                        tq.translate("createPost", "Internal database error", "description"), false);
        }
        if (Tools::captchaEnabled(boardName) && !Tools::isCaptchaValid(post.captcha)) {
            return bRet(error, tq.translate("createPost", "Invalid captcha", "error"), description,
                        tq.translate("createPost", "Captcha is missing or invalid", "description"), false);
        }
        if (dt.isValid() && post.files.isEmpty()) {
            return bRet(error, tq.translate("createPost", "No file", "error"), description,
                        tq.translate("createPost", "Attempt to create a thread without attaching a file",
                                     "description"), false);
        }
        if (post.text.isEmpty() && post.files.isEmpty()) {
            return bRet(error, tq.translate("createPost", "No file/text", "error"), description,
                        tq.translate("createPost", "Both file and comment are missing", "description"), false);
        }
        bool bump = post.email.compare("sage", Qt::CaseInsensitive);
        if (postLimit || bumpLimit) {
            odb::result<PostCount> pcr(db->query<PostCount>(odb::query<Post>::thread == thread->id()));
            const PostCount &postCount(*pcr.begin());
            if (postLimit && (postCount.count >= (int) postLimit)) {
                return bRet(error, tq.translate("createPost", "Post limit", "error"), description,
                            tq.translate("createPost", "The thread has reached it's post limit", "description"),
                            false);
            }
            if (bumpLimit && (postCount.count >= (int) bumpLimit))
                bump = false;
        }
        if (!dt.isValid())
            dt = QDateTime::currentDateTimeUtc();
        QByteArray hp = Tools::hashpass(req);
        QSharedPointer<Post> p(new Post(boardName, postNumber, dt, thread, Tools::userIp(req), post.password, hp));
        p->setEmail(post.email);
        QStringList fileNames;
        foreach (const Tools::File &f, post.files) {
            bool ok = false;
            fileNames << Tools::saveFile(f, boardName, &ok);
            if (!ok) {
                return bRet(error, tq.translate("createPost", "Internal error", "error"), description,
                            tq.translate("createPost", "Internal file system error", "description"), false);
            }
        }
        p->setFiles(fileNames);
        p->setName(post.name);
        p->setSubject(post.subject);
        p->setText(post.text);
        if (bump) {
            thread->setDateTime(dt);
            db->update(thread);
        }
        db->persist(p);
        return bRet(error, QString(), description, QString(), true);
    } catch (const odb::exception &e) {
        return bRet(error, tq.translate("createPost", "Internal error", "error"), description,
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
        bool trans = false;
        trans = !db;
        if (!db)
            db = createConnection();
        if (!db)
            return bRet(error, tq.translate("deletePostInternal", "Internal database error", "error"), false);
        QScopedPointer<odb::database> dbb;
        if (trans)
            dbb.reset(db);
        QScopedPointer<odb::transaction> transaction;
        if (trans)
            transaction.reset(new odb::transaction(db->begin()));
        odb::result<Thread> r(db->query<Thread>(odb::query<Thread>::board == boardName
                                                && odb::query<Thread>::number == postNumber));
        odb::result<Thread>::iterator i = r.begin();
        if (r.end() != i) {
            quint64 threadId = i->id();
            ++i;
            if (r.end() != i)
                return bRet(error, tq.translate("deletePostInternal", "Internal database error", "error"), false);
            odb::result<Post> rr(db->query<Post>(odb::query<Post>::thread == threadId));
            QStringList files;
            for (odb::result<Post>::iterator ii = rr.begin(); ii != rr.end(); ++ii)
                files += ii->files();
            db->erase_query<Post>(odb::query<Post>::thread == threadId);
            db->erase_query<Thread>(odb::query<Thread>::id == threadId);
            Tools::deleteFiles(boardName, files);
        } else {
            odb::result<Post> rr(db->query<Post>(odb::query<Post>::board == boardName
                                                 && odb::query<Post>::number == postNumber));
            odb::result<Post>::iterator ii = rr.begin();
            if (rr.end() == ii)
                return bRet(error, tq.translate("deletePostInternal", "No such post", "error"), false);
            QStringList files = ii->files();
            ++ii;
            if (rr.end() != ii)
                return bRet(error, tq.translate("deletePostInternal", "Internal database error", "error"), false);
            db->erase_query<Post>(odb::query<Post>::board == boardName && odb::query<Post>::number == postNumber);
            Tools::deleteFiles(boardName, files);
        }
        if (transaction)
            transaction->commit();
        return bRet(error, QString(), true);
    }  catch (const odb::exception &e) {
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

odb::database *createConnection()
{
    QString storagePath = Tools::storagePath();
    if (storagePath.isEmpty())
        return 0;
    QString fileName = storagePath + "/db.sqlite";
    if (!BDirTools::touch(fileName))
        return 0;
    return new odb::sqlite::database(Tools::toStd(fileName), SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE);
}

bool createPost(CreatePostParameters &p)
{
    TranslatorQt tq(p.locale);
    try {
        QScopedPointer<odb::database> db(createConnection());
        odb::transaction transaction(db->begin());
        QString err;
        QString desc;
        if (!createPostInternal(db.data(), p.request, p.params, p.files, p.bumpLimit, p.postLimit, &err, p.locale,
                                &desc)) {
            return bRet(p.error, err, p.description, desc, false);
        }
        transaction.commit();
        return bRet(p.error, QString(), p.description, QString(), true);
    } catch (const odb::exception &e) {
        return bRet(p.error, tq.translate("createPost", "Internal error", "error"), p.description,
                    Tools::fromStd(e.what()), false);
    }
}

void createSchema()
{
    QScopedPointer<odb::database> db(createConnection());
    if (!db)
        return;
    odb::connection_ptr c(db->connection());
    c->execute("PRAGMA foreign_keys=OFF");
    odb::transaction t(c->begin());
    odb::schema_catalog::create_schema(*db);
    t.commit();
    c->execute("PRAGMA foreign_keys=ON");
}

quint64 createThread(CreateThreadParameters &p)
{
    QString boardName = p.params.value("board");
    TranslatorQt tq(p.locale);
    try {
        QScopedPointer<odb::database> db(createConnection());
        odb::transaction transaction(db->begin());
        QString err;
        QString desc;
        quint64 postNumber = incrementPostCounter(db.data(), p.params.value("board"), &err, p.locale);
        if (!postNumber)
            return bRet(p.error, tq.translate("createThread", "Internal error", "error"), p.description, err, 0L);
        if (p.threadLimit) {
            odb::result<ThreadCount> tcr(db->query<ThreadCount>(odb::query<Thread>::board == boardName
                                                                && odb::query<Thread>::archived == false));
            const ThreadCount &threadCount(*tcr.begin());
            if (threadCount.count >= (int) p.threadLimit) {
                odb::result<ThreadIdDateTimeFixed> tir(db->query<ThreadIdDateTimeFixed>(
                                                           odb::query<Thread>::board == boardName
                                                           && odb::query<Thread>::archived == false));
                QList<ThreadIdDateTimeFixed> list;
                for (odb::result<ThreadIdDateTimeFixed>::iterator j = tir.begin(); j != tir.end(); ++j)
                    list << *j;
                qSort(list.begin(), list.end(), &threadIdDateTimeFixedLessThan);
                if (p.archiveLimit) {
                    odb::result<ThreadCount> atcr(db->query<ThreadCount>(odb::query<Thread>::board == boardName
                                                                         && odb::query<Thread>::archived == true));
                    const ThreadCount &archivedThreadCount(*atcr.begin());
                    if (archivedThreadCount.count >= (int) p.archiveLimit) {
                        odb::result<ThreadIdDateTimeFixed> atir(db->query<ThreadIdDateTimeFixed>(
                                                                    odb::query<Thread>::board == boardName
                                                                    && odb::query<Thread>::archived == true));
                        QList<ThreadIdDateTimeFixed> alist;
                        for (odb::result<ThreadIdDateTimeFixed>::iterator k = atir.begin(); k != atir.end(); ++k)
                            alist << *k;
                        qSort(alist.begin(), alist.end(), &threadIdDateTimeFixedLessThan);
                        if (!deletePostInternal(boardName, alist.last().number, p.description, p.locale, db.data()))
                            return bRet(p.error, tq.translate("createThread", "Internal error", "error"), 0L);
                    }
                    odb::result<Thread> tr(db->query<Thread>(odb::query<Thread>::id == list.last().id));
                    odb::result<Thread>::iterator k = tr.begin();
                    if (tr.end() == k) {
                        return bRet(p.error, tq.translate("createThread", "Internal error", "error"), p.description,
                                    tq.translate("createThread", "Internal database error", "error"), 0L);
                    }
                    Thread t = *k;
                    ++k;
                    if (tr.end() != k) {
                        return bRet(p.error, tq.translate("createThread", "Internal error", "error"), p.description,
                                    tq.translate("createThread", "Internal database error", "error"), 0L);
                    }
                    t.setArchived(true);
                    db->update(t);
                } else {
                    if (!deletePostInternal(boardName, list.last().number, p.description, p.locale, db.data()))
                        return bRet(p.error, tq.translate("createThread", "Internal error", "error"), 0L);
                }
            }
        }
        QDateTime dt = QDateTime::currentDateTimeUtc();
        QSharedPointer<Thread> thread(new Thread(p.params.value("board"), postNumber, dt));
        db->persist(thread);
        if (!createPostInternal(db.data(), p.request, p.params, p.files, 0, 0, &err, p.locale, &desc, dt, postNumber))
            return bRet(p.error, err, p.description, desc, 0L);
        transaction.commit();
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
        QScopedPointer<odb::database> db(createConnection());
        if (!db)
            return bRet(error, tq.translate("deletePost", "Internal database error", "error"), false);
        odb::transaction transaction(db->begin());
        odb::result<Post> r(db->query<Post>(odb::query<Post>::board == boardName
                                            && odb::query<Post>::number == postNumber));
        odb::result<Post>::iterator i = r.begin();
        if (r.end() == i)
            return bRet(error, tq.translate("deletePost", "No such post", "error"), false);
        QByteArray ppwd = i->password();
        QByteArray phps = i->hashpass();
        ++i;
        if (r.end() != i)
            return bRet(error, tq.translate("deletePost", "Internal database error", "error"), false);
        if (password.isEmpty()) {
            if (hashpass != phps) {
                int lvl = registeredUserLevel(req, false);
                if (lvl < RegisteredUser::ModerLevel || registeredUserLevel(phps, false) >= lvl)
                    return bRet(error, tq.translate("deletePost", "Not enough rights", "error"), false);
                QStringList boards = registeredUserBoards(req, false);
                if (!boards.contains("*") && !boards.contains(boardName))
                    return bRet(error, tq.translate("deletePost", "Not enough rights", "error"), false);
            }
        } else if (password != ppwd) {
            return bRet(error, tq.translate("deletePost", "Incorrect password", "error"), false);
        }
        if (!deletePostInternal(boardName, postNumber, error, tq.locale(), db.data()))
            return false;
        transaction.commit();
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
        odb::result<PostCounter> pcr = db->query<PostCounter>(odb::query<PostCounter>::board == boardName);
        odb::result<PostCounter>::iterator pci = pcr.begin();
        if (pcr.end() == pci) {
            PostCounter counter(boardName);
            db->persist(counter);
            pcr = db->query<PostCounter>(odb::query<PostCounter>::board == boardName);
            pci = pcr.begin();
        }
        if (pcr.end() == pci)
            return bRet(error, tq.translate("incrementPostCounter", "Internal database error", "error"), 0L);
        PostCounter counter = *pci;
        ++pci;
        if (pcr.end() != pci)
            return bRet(error, tq.translate("incrementPostCounter", "Internal database error", "error"), 0L);
        incremented = counter.incrementLastPostNumber();
        db->update(counter);
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
        odb::result<PostCounter> pcr = db->query<PostCounter>(odb::query<PostCounter>::board == boardName);
        odb::result<PostCounter>::iterator pci = pcr.begin();
        if (pcr.end() == pci) {
            PostCounter counter(boardName);
            db->persist(counter);
            pcr = db->query<PostCounter>(odb::query<PostCounter>::board == boardName);
            pci = pcr.begin();
        }
        if (pcr.end() == pci)
            return bRet(error, tq.translate("lastPostNumber", "Internal database error", "error"), 0L);
        PostCounter counter = *pci;
        ++pci;
        if (pcr.end() != pci)
            return bRet(error, tq.translate("lastPostNumber", "Internal database error", "error"), 0L);
        return bRet(error, QString(), counter.lastPostNumber());
    } catch (const odb::exception &e) {
        return bRet(error, Tools::fromStd(e.what()), 0L);
    }
}

QString posterIp(const QString &boardName, quint64 postNumber)
{
    try {
        QScopedPointer<odb::database> db(createConnection());
        if (!db)
            return "";
        odb::transaction transaction(db->begin());
        odb::result<Post> r(db->query<Post>(odb::query<Post>::board == boardName
                                            && odb::query<Post>::number == postNumber));
        odb::result<Post>::iterator i = r.begin();
        if (r.end() == i)
            return "";
        QString ip = i->posterIp();
        ++i;
        if (r.end() != i)
            return "";
        transaction.commit();
        return ip;
    }  catch (const odb::exception &e) {
        qDebug() << e.what();
        return "";
    }
}

QStringList registeredUserBoards(const cppcms::http::request &req, bool trans)
{
    QByteArray hp = Tools::hashpass(req);
    if (hp.isEmpty())
        return QStringList();
    return registeredUserBoards(hp, trans);
}

QStringList registeredUserBoards(const QByteArray &hashpass, bool trans)
{
    bool b = false;
    Tools::toString(hashpass, &b);
    if (!b)
        return QStringList();
    try {
        QScopedPointer<odb::database> db(createConnection());
        if (!db)
            return QStringList();
        QScopedPointer<odb::transaction> transaction;
        if (trans)
            transaction.reset(new odb::transaction(db->begin()));
        odb::result<RegisteredUser> r(db->query<RegisteredUser>(odb::query<RegisteredUser>::hashpass == hashpass));
        odb::result<RegisteredUser>::iterator i = r.begin();
        if (r.end() == i)
            return QStringList();
        QStringList boards = i->boards();
        ++i;
        if (r.end() != i)
            return QStringList();
        if (trans)
            transaction->commit();
        return boards;
    }  catch (const odb::exception &e) {
        qDebug() << e.what();
        return QStringList();
    }
}

int registeredUserLevel(const cppcms::http::request &req, bool trans)
{
    QByteArray hp = Tools::hashpass(req);
    if (hp.isEmpty())
        return -1;
    return registeredUserLevel(hp, trans);
}

int registeredUserLevel(const QByteArray &hashpass, bool trans)
{
    bool b = false;
    Tools::toString(hashpass, &b);
    if (!b)
        return -1;
    try {
        QScopedPointer<odb::database> db(createConnection());
        if (!db)
            return -1;
        QScopedPointer<odb::transaction> transaction;
        if (trans)
            transaction.reset(new odb::transaction(db->begin()));
        odb::result<RegisteredUserLevel> r(db->query<RegisteredUserLevel>(
                                               odb::query<RegisteredUser>::hashpass == hashpass));
        odb::result<RegisteredUserLevel>::iterator i = r.begin();
        if (r.end() == i)
            return -1;
        int level = i->level;
        ++i;
        if (r.end() != i)
            return -1;
        if (trans)
            transaction->commit();
        return level;
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
        QScopedPointer<odb::database> db(createConnection());
        if (!db)
            return bRet(error, tq.translate("registerUser", "Internal database error", "error"), false);
        odb::transaction transaction(db->begin());
        RegisteredUser user(hashpass, QDateTime::currentDateTimeUtc(), level, boards);
        db->persist(user);
        transaction.commit();
        return bRet(error, QString(), true);
    } catch (const odb::exception &e) {
        return bRet(error, Tools::fromStd(e.what()), false);
    }
}

bool setThreadFixed(const QString &board, quint64 threadNumber, bool fixed, QString *error, const QLocale &l)
{
    TranslatorQt tq(l);
    try {
        QScopedPointer<odb::database> db(createConnection());
        if (!db)
            return bRet(error, tq.translate("setThreadFixed", "Internal database error", "error"), false);
        odb::transaction transaction(db->begin());
        odb::result<Thread> tr = db->query<Thread>(odb::query<Thread>::number == threadNumber
                                                   && odb::query<Thread>::board == board);
        odb::result<Thread>::iterator ti = tr.begin();
        if (tr.end() == ti)
            return bRet(error, tq.translate("setThreadFixed", "No such thread", "error"), false);
        QSharedPointer<Thread> thread(new Thread(*ti));
        ++ti;
        if (tr.end() != ti)
            return bRet(error, tq.translate("setThreadFixed", "Internal database error", "error"), false);
        if (thread->fixed() == fixed)
            return bRet(error, QString(), true);
        thread->setFixed(fixed);
        db->update(thread);
        transaction.commit();
        return bRet(error, QString(), true);
    } catch (const odb::exception &e) {
        return bRet(error, Tools::fromStd(e.what()), false);
    }
}

bool setThreadOpened(const QString &board, quint64 threadNumber, bool opened, QString *error, const QLocale &l)
{
    TranslatorQt tq(l);
    try {
        QScopedPointer<odb::database> db(createConnection());
        if (!db)
            return bRet(error, tq.translate("setThreadOpened", "Internal database error", "error"), false);
        odb::transaction transaction(db->begin());
        odb::result<Thread> tr = db->query<Thread>(odb::query<Thread>::number == threadNumber
                                                   && odb::query<Thread>::board == board);
        odb::result<Thread>::iterator ti = tr.begin();
        if (tr.end() == ti)
            return bRet(error, tq.translate("setThreadOpened", "No such thread", "error"), false);
        QSharedPointer<Thread> thread(new Thread(*ti));
        ++ti;
        if (tr.end() != ti)
            return bRet(error, tq.translate("setThreadOpened", "No such thread", "error"), false);
        if (thread->postingEnabled() == opened)
            return bRet(error, QString(), true);
        thread->setPostingEnabled(opened);
        db->update(thread);
        transaction.commit();
        return bRet(error, QString(), true);
    } catch (const odb::exception &e) {
        return bRet(error, Tools::fromStd(e.what()), false);
    }
}

}
