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

namespace Database
{

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
OLOLORD_EXPORT void createSchema();
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
OLOLORD_EXPORT bool setThreadFixed(const QString &board, quint64 threadNumber, bool fixed, QString *error = 0,
                                   const QLocale &l = BCoreApplication::locale());
OLOLORD_EXPORT bool setThreadOpened(const QString &board, quint64 threadNumber, bool opened, QString *error = 0,
                                    const QLocale &l = BCoreApplication::locale());

}

#endif // DATABASE_H
