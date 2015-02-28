#ifndef CONTROLLER_H
#define CONTROLLER_H

class AbstractBoard;
//class Post;

namespace Content
{

class Base;
class BaseBoard;
class Post;

}

namespace Database
{

class BanInfo;

}

class QLocale;

namespace cppcms
{

class application;

namespace http
{

class request;

}

}

#include "../global.h"
//#include "baseboard.h"
#include "tools.h"

#include <QDateTime>
#include <QList>
#include <QString>

#include <string>

namespace Controller
{

enum RequestType
{
    GetRequest = 0x01,
    PostRequest = 0x02
};

enum UserActionType
{
    ReadAction = 10,
    WriteAction = 1
};

OLOLORD_EXPORT QList<Content::Post> getNewPosts(const cppcms::http::request &req, const QString &boardName,
    quint64 threadNumber, quint64 lastPostNumber, bool *ok = 0, QString *error = 0);
OLOLORD_EXPORT Content::Post getPost(const cppcms::http::request &req, const QString &boardName, quint64 postNumber,
                                     bool *ok = 0, QString *error = 0);
OLOLORD_EXPORT void initBase(Content::Base &c, const cppcms::http::request &req, const QString &pageTitle = QString());
OLOLORD_EXPORT void initBaseBoard(Content::BaseBoard &c, const cppcms::http::request &req, const AbstractBoard *board,
    bool postingEnabled, const QString &pageTitle = QString(), quint64 currentThread = 0);
OLOLORD_EXPORT std::string processPostText(QString text, const QString &boardName, quint64 threadNumber,
                                           bool processCode);
OLOLORD_EXPORT void redirect(cppcms::application &app, const QString &where);
OLOLORD_EXPORT void renderBan(cppcms::application &app, const Database::BanInfo &info);
OLOLORD_EXPORT void renderError(cppcms::application &app, const QString &error,
                                const QString &description = QString());
OLOLORD_EXPORT void renderIpBan(cppcms::application &app, int level);
OLOLORD_EXPORT void renderNotFound(cppcms::application &app);
OLOLORD_EXPORT bool testBan(cppcms::application &app, UserActionType proposedAction, const QString &board);
OLOLORD_EXPORT bool testParams(const AbstractBoard *board, cppcms::application &app,
                               const Tools::PostParameters &params, const Tools::FileList &files, bool post);
OLOLORD_EXPORT bool testRequest(cppcms::application &app, int acceptedTypes);
OLOLORD_EXPORT QString toHtml(const QString &s);
OLOLORD_EXPORT void toHtml(QString *s);
/*OLOLORD_EXPORT Content::Post toController(const Post &post, const AbstractBoard *board, const QLocale &l,
                                          const cppcms::http::request &req);*/

}

#endif // CONTROLLER_H
