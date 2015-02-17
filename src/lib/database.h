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
#include <QList>
#include <QMap>
#include <QString>
#include <QStringList>

#ifndef OLOLORD_NO_ODB
#include <odb/database.hxx>
#include <odb/query.hxx>
#endif

namespace Database
{

#ifndef OLOLORD_NO_ODB
template <typename ResultType> class Result
{
public:
    ResultType * const data;
    const bool error;
public:
    explicit Result(bool err = true) :
        data(0), error(err)
    {
        //
    }
    explicit Result(const odb::result_iterator<ResultType> &i) :
        data(new ResultType(*i)), error(false)
    {
        //
    }
    ~Result()
    {
        delete data;
    }
public:
    ResultType *operator ->() const
    {
        return data;
    }
};

template <typename ResultType, typename QueryType> QList<ResultType> query(
        odb::database *db, const odb::query<QueryType> &q, bool *ok = 0)
{
    if (!db)
        return bRet(ok, false, QList<ResultType>());
    odb::result<ResultType> r(db->query(q));
    QList<ResultType> list;
    for (odb::result_iterator<ResultType> i = r.begin(); i != r.end(); ++i)
        list << *i;
    return list;
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
    return list;
}

template <typename ResultType> QList<ResultType> queryAll(odb::database *db, bool *ok = 0)
{
    if (!db)
        return bRet(ok, false, QList<ResultType>());
    odb::result<ResultType> r(db->query<ResultType>());
    QList<ResultType> list;
    for (odb::result_iterator<ResultType> i = r.begin(); i != r.end(); ++i)
        list << *i;
    return list;
}

template <typename ResultType, typename QueryType> Result<ResultType> queryOne(odb::database *db,
                                                                               const odb::query<QueryType> &q)
{
    if (!db)
        return Result<ResultType>();
    odb::result<ResultType> r(db->query(q));
    odb::result_iterator<ResultType> i = r.begin();
    if (r.end() == i)
        return Result<ResultType>(false);
    Result<ResultType> v(i);
    ++i;
    if (r.end() != i)
        return Result<ResultType>();
    return v;
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
OLOLORD_EXPORT QStringList registeredUserBoards(const cppcms::http::request &req, bool transaction = true);
OLOLORD_EXPORT QStringList registeredUserBoards(const QByteArray &hashpass, bool transaction = false);
OLOLORD_EXPORT int registeredUserLevel(const cppcms::http::request &req, bool transaction = true);
OLOLORD_EXPORT int registeredUserLevel(const QByteArray &hashpass, bool transaction = false);
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

}

#endif // DATABASE_H
