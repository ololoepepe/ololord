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
        odb::database *db = createConnection();
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
            user = QSharedPointer<BannedUser>(new BannedUser(board, ip, dt));
        } else {
            user = QSharedPointer<BannedUser>(new BannedUser(*i));
            ++i;
            if (r.end() != i)
                return bRet(error, tq.translate("banUserInternal", "Internal database error", "error"), false);
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

static bool createPostInternal(odb::database *db, const cppcms::http::request &req, unsigned int bumpLimit,
                               unsigned int postLimit, QString *error, const QLocale &l, QString *description,
                               QDateTime dt = QDateTime(), quint64 threadNumber = 0L)
{
    Tools::PostParameters params = Tools::postParameters(req);
    QString boardName = params.value("board");
    if (!threadNumber)
        threadNumber = params.value("thread").toULongLong();
    Tools::Post post = Tools::toPost(req);
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
        QString hp = Tools::hashPassString(req);
        QByteArray hpba = Tools::toHashpass(hp);
        QSharedPointer<Post> p(new Post(boardName, postNumber, dt, thread, Tools::userIp(req), post.password, hpba));
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
        p->setFiles(BeQt::serialize(fileNames));
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

bool createPost(const cppcms::http::request &req, unsigned int bumpLimit, unsigned int postLimit, QString *error,
                const QLocale &l, QString *description)
{
    TranslatorQt tq(l);
    try {
        odb::database *db = createConnection();
        odb::transaction transaction(db->begin());
        QString err;
        QString desc;
        if (!createPostInternal(db, req, bumpLimit, postLimit, &err, l, &desc))
            return bRet(error, err, description, desc, false);
        transaction.commit();
        return bRet(error, QString(), description, QString(), true);
    } catch (const odb::exception &e) {
        return bRet(error, tq.translate("createPost", "Internal error", "error"), description,
                    Tools::fromStd(e.what()), false);
    }
}

void createSchema()
{
    odb::database *db = createConnection();
    if (!db)
        return;
    odb::connection_ptr c(db->connection());
    c->execute("PRAGMA foreign_keys=OFF");
    odb::transaction t(c->begin());
    odb::schema_catalog::create_schema(*db);
    t.commit();
    c->execute("PRAGMA foreign_keys=ON");
}

quint64 createThread(const cppcms::http::request &req, unsigned int threadLimit, QString *error, const QLocale &l,
                     QString *description)
{
    Tools::PostParameters params = Tools::postParameters(req);
    QString boardName = params.value("board");
    TranslatorQt tq(l);
    try {
        odb::database *db = createConnection();
        odb::transaction transaction(db->begin());
        QString err;
        QString desc;
        quint64 postNumber = incrementPostCounter(db, params.value("board"), &err, tq.locale());
        if (!postNumber)
            return bRet(error, tq.translate("createThread", "Internal error", "error"), description, err, 0L);
        if (threadLimit) {
            odb::result<ThreadCount> tcr(db->query<ThreadCount>(odb::query<Thread>::board == boardName));
            const ThreadCount &threadCount(*tcr.begin());
            if (threadCount.count >= (int) threadLimit) {
                odb::result<ThreadIdDateTimeFixed> tir(db->query<ThreadIdDateTimeFixed>(
                                                           odb::query<Thread>::board == boardName));
                QList<ThreadIdDateTimeFixed> list;
                for (odb::result<ThreadIdDateTimeFixed>::iterator j = tir.begin(); j != tir.end(); ++j)
                    list << *j;
                qSort(list.begin(), list.end(), &threadIdDateTimeFixedLessThan);
                db->erase_query<Post>(odb::query<Post>::thread == list.last().id);
                db->erase<Thread>(list.last().id);
            }
        }
        QDateTime dt = QDateTime::currentDateTimeUtc();
        QSharedPointer<Thread> thread(new Thread(params.value("board"), postNumber, dt));
        db->persist(thread);
        if (!createPostInternal(db, req, 0, 0, &err, l, &desc, dt, postNumber))
            return bRet(error, err, description, desc, 0L);
        transaction.commit();
        return bRet(error, QString(), description, QString(), postNumber);
    } catch (const odb::exception &e) {
        return bRet(error, tq.translate("createThread", "Internal error", "error"), description,
                    Tools::fromStd(e.what()), 0L);
    }
}

bool deletePost(const QString &boardName, quint64 postNumber, QString *error, const QLocale &l)
{
    TranslatorQt tq(l);
    if (boardName.isEmpty() || !AbstractBoard::boardNames().contains(boardName))
        return bRet(error, tq.translate("deletePost", "Invalid board name", "error"), false);
    if (!postNumber)
        return bRet(error, tq.translate("deletePost", "Invalid post number", "error"), false);
    try {
        odb::database *db = createConnection();
        if (!db)
            return bRet(error, tq.translate("deletePost", "Internal database error", "error"), false);
        odb::transaction transaction(db->begin());
        odb::result<Thread> r(db->query<Thread>(odb::query<Thread>::board == boardName
                                                && odb::query<Thread>::number == postNumber));
        odb::result<Thread>::iterator i = r.begin();
        if (r.end() != i) {
            quint64 threadId = i->id();
            ++i;
            if (r.end() != i)
                return bRet(error, tq.translate("deletePost", "Internal database error", "error"), false);
            db->erase_query<Post>(odb::query<Post>::thread == threadId);
            db->erase_query<Thread>(odb::query<Thread>::id == threadId);
            //TODO: Delete files
        } else {
            odb::result<Post> rr(db->query<Post>(odb::query<Post>::board == boardName
                                                 && odb::query<Post>::number == postNumber));
            odb::result<Post>::iterator ii = rr.begin();
            if (rr.end() == ii)
                return bRet(error, tq.translate("deletePost", "No such post", "error"), false);
            ++ii;
            if (rr.end() != ii)
                return bRet(error, tq.translate("deletePost", "Internal database error", "error"), false);
            db->erase_query<Post>(odb::query<Post>::board == boardName && odb::query<Post>::number == postNumber);
            //TODO: Delete files
        }
        transaction.commit();
        return bRet(error, QString(), true);
    }  catch (const odb::exception &e) {
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

bool mayDeletePost(const QString &boardName, quint64 postNumber, const QString &password, QString *error,
                   const QLocale &l)
{
    TranslatorQt tq(l);
    if (!AbstractBoard::boardNames().contains(boardName))
        return bRet(error, tq.translate("mayDeletePost", "Invalid board name", "error"), false);
    if (!postNumber)
        return bRet(error, tq.translate("mayDeletePost", "Invalid post number", "error"), false);
    if (password.isEmpty())
        return bRet(error, tq.translate("mayDeletePost", "Invalid password", "error"), false);
    try {
        odb::database *db = createConnection();
        if (!db)
            return bRet(error, tq.translate("mayDeletePost", "Internal database error", "error"), false);
        odb::transaction transaction(db->begin());
        odb::result<Post> r(db->query<Post>(odb::query<Post>::board == boardName
                                            && odb::query<Post>::number == postNumber));
        odb::result<Post>::iterator i = r.begin();
        if (r.end() == i)
            return bRet(error, tq.translate("mayDeletePost", "No such post", "error"), false);
        QByteArray ppwd = i->password();
        ++i;
        if (r.end() != i)
            return bRet(error, tq.translate("mayDeletePost", "Internal database error", "error"), false);
        if (QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha1) != ppwd)
            return bRet(error, tq.translate("mayDeletePost", "Incorrect password", "error"), false);
        transaction.commit();
        return bRet(error, QString(), true);
    } catch (const odb::exception &e) {
        return bRet(error, Tools::fromStd(e.what()), false);
    }
}

QString posterIp(const QString &boardName, quint64 postNumber)
{
    try {
        odb::database *db = createConnection();
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

int registeredUserLevel(const cppcms::http::request &req)
{
    QString hp = Tools::hashPassString(req);
    QByteArray hpba = Tools::toHashpass(hp);
    if (hpba.isEmpty())
        return -1;
    return registeredUserLevel(hpba, true);
}

int registeredUserLevel(const QByteArray &hashpass, bool trans)
{
    bool b = false;
    Tools::toString(hashpass, &b);
    if (!b)
        return -1;
    try {
        odb::database *db = createConnection();
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

bool registerUser(const QByteArray &hashpass, RegisteredUser::Level level, QString *error, const QLocale &l)
{
    bool b = false;
    Tools::toString(hashpass, &b);
    TranslatorQt tq(l);
    if (!b)
        return bRet(error, tq.translate("registerUser", "Invalid hashpass", "error"), false);
    try {
        odb::database *db = createConnection();
        if (!db)
            return bRet(error, tq.translate("registerUser", "Internal database error", "error"), false);
        odb::transaction transaction(db->begin());
        RegisteredUser user(hashpass, QDateTime::currentDateTimeUtc(), level);
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
        odb::database *db = createConnection();
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
        odb::database *db = createConnection();
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
