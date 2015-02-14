#ifndef DATABASE_H
#define DATABASE_H

namespace Tools
{

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
#include <QString>

namespace Database
{

OLOLORD_EXPORT bool banUser(const QString &ip, const QString &board = "*", int level = 1,
                            const QString &reason = QString(), const QDateTime &expires = QDateTime(),
                            QString *error = 0, const QLocale &l = BCoreApplication::locale());
OLOLORD_EXPORT bool banUser(const QString &sourceBoard, quint64 postNumber, const QString &board = "*", int level = 1,
                            const QString &reason = QString(), const QDateTime &expires = QDateTime(),
                            QString *error = 0, const QLocale &l = BCoreApplication::locale());
OLOLORD_EXPORT odb::database *createConnection();
OLOLORD_EXPORT bool createPost(const cppcms::http::request &req, unsigned int bumpLimit, unsigned int postLimit,
                               QString *error = 0, const QLocale &l = BCoreApplication::locale(),
                               QString *description = 0);
OLOLORD_EXPORT void createSchema();
OLOLORD_EXPORT quint64 createThread(const cppcms::http::request &req, unsigned int threadLimit, QString *error = 0,
                                    const QLocale &l = BCoreApplication::locale(), QString *description = 0);
OLOLORD_EXPORT quint64 incrementPostCounter(odb::database *db, const QString &boardName, QString *error = 0,
                                            const QLocale &l = BCoreApplication::locale());
OLOLORD_EXPORT quint64 lastPostNumber(odb::database *db, const QString &boardName, QString *error = 0,
                                      const QLocale &l = BCoreApplication::locale());
OLOLORD_EXPORT QString posterIp(const QString &boardName, quint64 postNumber);
OLOLORD_EXPORT int registeredUserLevel(const cppcms::http::request &req);
OLOLORD_EXPORT int registeredUserLevel(const QByteArray &hashpass, bool transaction = false);
OLOLORD_EXPORT bool registerUser(const QByteArray &hashpass, RegisteredUser::Level level, QString *error,
                                 const QLocale &l = BCoreApplication::locale());
OLOLORD_EXPORT bool setThreadFixed(const QString &board, quint64 threadNumber, bool fixed, QString *error = 0,
                                   const QLocale &l = BCoreApplication::locale());
OLOLORD_EXPORT bool setThreadOpened(const QString &board, quint64 threadNumber, bool opened, QString *error = 0,
                                    const QLocale &l = BCoreApplication::locale());

}

#endif // DATABASE_H
