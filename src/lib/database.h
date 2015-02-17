#ifndef DATABASE_H
#define DATABASE_H

namespace Tools
{

class File;
class Post;

}

class QLocale;

namespace cppcms
{

namespace http
{

class request;

}

}

namespace odb
{

class database;

}

#include "global.h"
#include "stored/registereduser.h"

#include <BCoreApplication>

#include <QDateTime>
#include <QDebug>
#include <QList>
#include <QMap>
#include <QSharedPointer>
#include <QString>
#include <QStringList>

#ifndef OLOLORD_NO_ODB
#include <odb/database.hxx>
#include <odb/query.hxx>
#include <odb/transaction.hxx>
#endif

namespace Database
{

#ifndef OLOLORD_NO_ODB
template <typename ResultType> class Result
{
public:
    const bool error;
public:
    QSharedPointer<ResultType> data;
public:
    explicit Result(bool err = true) :
        error(err)
    {
        //
    }
    explicit Result(const odb::result_iterator<ResultType> &i) :
        error(false)
    {
        data = QSharedPointer<ResultType>(new ResultType(*i));
    }
    Result(const Result<ResultType> &other) :
        error(other.error)
    {
        *this = other;
    }
public:
    void clear()
    {
        data.clear();
    }
public:
    ResultType *operator ->() const
    {
        return data.data();
    }
    ResultType &operator *() const
    {
        return *data;
    }
    bool operator !() const
    {
        return data.isNull();
    }
    Result<ResultType> &operator =(const Result<ResultType> &other)
    {
        data = other.data;
        *const_cast<bool *>(&error) = other.error;
        return *this;
    }
    Result<ResultType> &operator =(ResultType *t)
    {
        data = QSharedPointer<ResultType>(t);
        return *this;
    }
    operator bool() const
    {
        return !data.isNull();
    }
};

template <typename ResultType, typename QueryType> QList<ResultType> query(
        odb::database *db, const odb::query<QueryType> &q, bool *ok = 0)
{
    if (!db)
        return bRet(ok, false, QList<ResultType>());
    odb::result<ResultType> r(db->query<ResultType>(q));
    QList<ResultType> list;
    for (odb::result_iterator<ResultType> i = r.begin(); i != r.end(); ++i)
        list << *i;
    return bRet(ok, true, list);
}

template <typename ResultType, typename QueryType> QList<ResultType> query(odb::database *db, const QString &q,
                                                                           bool *ok = 0)
{
    if (!db)
        return bRet(ok, false, QList<ResultType>());
    odb::result<ResultType> r(db->query<ResultType>(q.toUtf8().constData()));
    QList<ResultType> list;
    for (odb::result_iterator<ResultType> i = r.begin(); i != r.end(); ++i)
        list << *i;
    return bRet(ok, true, list);
}

template <typename ResultType> QList<ResultType> queryAll(odb::database *db, bool *ok = 0)
{
    if (!db)
        return bRet(ok, false, QList<ResultType>());
    odb::result<ResultType> r(db->query<ResultType>());
    QList<ResultType> list;
    for (odb::result_iterator<ResultType> i = r.begin(); i != r.end(); ++i)
        list << *i;
    return bRet(ok, true, list);
}

template <typename ResultType, typename QueryType> Result<ResultType> queryOne(odb::database *db,
                                                                               const odb::query<QueryType> &q)
{
    if (!db)
        return Result<ResultType>();
    odb::result<ResultType> r(db->query<ResultType>(q));
    odb::result_iterator<ResultType> i = r.begin();
    if (r.end() == i)
        return Result<ResultType>(false);
    Result<ResultType> v(i);
    ++i;
    if (r.end() != i)
        return Result<ResultType>(false);
    return v;
}

template <typename T> void persist(odb::database *db, const Result<T> &t, bool *ok = 0)
{
    if (!db || !t)
        return bSet(ok, false);
    db->persist(*t);
    bSet(ok, true);
}

template <typename T> void update(odb::database *db, const Result<T> &t, bool *ok = 0)
{
    if (!db || !t)
        return bSet(ok, false);
    db->update(*t);
    bSet(ok, true);
}

template <typename T> void erase(odb::database *db, const Result<T> &t, bool *ok = 0)
{
    if (!db || !t)
        return bSet(ok, false);
    db->erase(*t);
    bSet(ok, true);
}
#endif

struct OLOLORD_EXPORT CreatePostParameters
{
    const QList<Tools::File> &files;
    const QLocale &locale;
    const QMap<QString, QString> &params;
    const cppcms::http::request &request;
public:
    unsigned int bumpLimit;
    unsigned int postLimit;
    QString *error;
    QString *description;
public:
    explicit CreatePostParameters(const cppcms::http::request &req, const QMap<QString, QString> &ps,
                                   const QList<Tools::File> &fs, const QLocale &l = BCoreApplication::locale()) :
        files(fs), locale(l), params(ps), request(req)
    {
        bumpLimit = 0;
        postLimit = 0;
        error = 0;
        description = 0;
    }
};

struct OLOLORD_EXPORT CreateThreadParameters
{
    const QList<Tools::File> &files;
    const QLocale &locale;
    const QMap<QString, QString> &params;
    const cppcms::http::request &request;
public:
    unsigned int archiveLimit;
    unsigned int threadLimit;
    QString *error;
    QString *description;
public:
    explicit CreateThreadParameters(const cppcms::http::request &req, const QMap<QString, QString> &ps,
                                    const QList<Tools::File> &fs, const QLocale &l = BCoreApplication::locale()) :
        files(fs), locale(l), params(ps), request(req)
    {
        threadLimit = 0;
        error = 0;
        description = 0;
    }
};

OLOLORD_EXPORT bool banUser(const QString &ip, const QString &board = "*", int level = 1,
                            const QString &reason = QString(), const QDateTime &expires = QDateTime(),
                            QString *error = 0, const QLocale &l = BCoreApplication::locale());
OLOLORD_EXPORT bool banUser(const QString &sourceBoard, quint64 postNumber, const QString &board = "*", int level = 1,
                            const QString &reason = QString(), const QDateTime &expires = QDateTime(),
                            QString *error = 0, const QLocale &l = BCoreApplication::locale());
OLOLORD_EXPORT void checkOutdatedEntries();
OLOLORD_EXPORT odb::database *createConnection();
OLOLORD_EXPORT bool createPost(CreatePostParameters &p);
OLOLORD_EXPORT quint64 createThread(CreateThreadParameters &p);
OLOLORD_EXPORT bool deletePost(const QString &boardName, quint64 postNumber, QString *error = 0,
                               const QLocale &l = BCoreApplication::locale());
OLOLORD_EXPORT bool deletePost(const QString &boardName, quint64 postNumber,  const cppcms::http::request &req,
                               const QByteArray &password, QString *error = 0);
OLOLORD_EXPORT quint64 incrementPostCounter(odb::database *db, const QString &boardName, QString *error = 0,
                                            const QLocale &l = BCoreApplication::locale());
OLOLORD_EXPORT quint64 lastPostNumber(odb::database *db, const QString &boardName, QString *error = 0,
                                      const QLocale &l = BCoreApplication::locale());
OLOLORD_EXPORT QString posterIp(const QString &boardName, quint64 postNumber);
OLOLORD_EXPORT QStringList registeredUserBoards(const cppcms::http::request &req, odb::database *db = 0);
OLOLORD_EXPORT QStringList registeredUserBoards(const QByteArray &hashpass, odb::database *db = 0);
OLOLORD_EXPORT int registeredUserLevel(const cppcms::http::request &req, odb::database *db = 0);
OLOLORD_EXPORT int registeredUserLevel(const QByteArray &hashpass, odb::database *db = 0);
OLOLORD_EXPORT bool registerUser(const QByteArray &hashpass, RegisteredUser::Level level = RegisteredUser::UserLevel,
                                 const QStringList &boards = QStringList("*"), QString *error = 0,
                                 const QLocale &l = BCoreApplication::locale());
OLOLORD_EXPORT bool setThreadFixed(const QString &boardName, quint64 threadNumber, bool fixed, QString *error = 0,
                                   const QLocale &l = BCoreApplication::locale());
OLOLORD_EXPORT bool setThreadFixed(const QString &boardName, quint64 threadNumber, bool fixed,
                                   const cppcms::http::request &req, QString *error = 0);
OLOLORD_EXPORT bool setThreadOpened(const QString &boardName, quint64 threadNumber, bool opened, QString *error = 0,
                                    const QLocale &l = BCoreApplication::locale());
OLOLORD_EXPORT bool setThreadOpened(const QString &boardName, quint64 threadNumber, bool opened,
                                    const cppcms::http::request &req, QString *error = 0);

#ifndef OLOLORD_NO_ODB
class OLOLORD_EXPORT Transaction
{
private:
    const bool external;
private:
    odb::database *mdb;
    odb::transaction *transaction;
public:
    explicit Transaction(odb::database *db = 0) :
        external(db)
    {
        transaction = 0;
        mdb = db;
        if (external)
            return;
        mdb = createConnection();
        if (!mdb)
            return;
        transaction = new odb::transaction(mdb->begin());
    }
    ~Transaction()
    {
        rollback();
        if (!external && mdb) {
            delete mdb;
        }
    }
public:
    void commit()
    {
        if (external || !transaction)
            return;
        transaction->commit();
        delete transaction;
        transaction = 0;
    }
    odb::database *db() const
    {
        return mdb;
    }
    void restart()
    {
        if (external || mdb || transaction)
            return;
        transaction = new odb::transaction(mdb->begin());
    }
    void rollback()
    {
        if (external || !transaction)
            return;
        transaction->rollback();
        delete transaction;
        transaction = 0;
    }
};
#endif

}

#endif // DATABASE_H
