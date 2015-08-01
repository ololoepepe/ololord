#ifndef CONTROLLER_H
#define CONTROLLER_H

class AbstractBoard;

namespace Content
{

class Base;
class BaseBoard;

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

#include "global.h"
#include "tools.h"

#include <QDateTime>
#include <QList>
#include <QString>

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

OLOLORD_EXPORT void initBase(Content::Base &c, const cppcms::http::request &req, const QString &pageTitle = QString());
OLOLORD_EXPORT bool initBaseBoard(Content::BaseBoard &c, const cppcms::http::request &req, const AbstractBoard *board,
    bool postingEnabled, const QString &pageTitle = QString(), quint64 currentThread = 0);
OLOLORD_EXPORT void redirect(cppcms::application &app, const QString &where);
OLOLORD_EXPORT void renderBan(cppcms::application &app, const Database::BanInfo &info);
OLOLORD_EXPORT void renderBanAjax(cppcms::application &app, const Database::BanInfo &info);
OLOLORD_EXPORT void renderBanNonAjax(cppcms::application &app, const Database::BanInfo &info);
OLOLORD_EXPORT void renderError(cppcms::application &app, const QString &error,
                                const QString &description = QString());
OLOLORD_EXPORT void renderErrorAjax(cppcms::application &app, const QString &error,
                                    const QString &description = QString());
OLOLORD_EXPORT void renderErrorNonAjax(cppcms::application &app, const QString &error,
                                       const QString &description = QString());
OLOLORD_EXPORT void renderIpBan(cppcms::application &app, int level);
OLOLORD_EXPORT void renderIpBanAjax(cppcms::application &app, int level);
OLOLORD_EXPORT void renderIpBanNonAjax(cppcms::application &app, int level);
OLOLORD_EXPORT void renderNotFound(cppcms::application &app);
OLOLORD_EXPORT void renderNotFoundAjax(cppcms::application &app);
OLOLORD_EXPORT void renderNotFoundNonAjax(cppcms::application &app);
OLOLORD_EXPORT void renderSuccessfulPostAjax(cppcms::application &app, quint64 postNumber);
OLOLORD_EXPORT void renderSuccessfulThreadAjax(cppcms::application &app, quint64 threadNumber);
OLOLORD_EXPORT bool shouldBeAjax(cppcms::application &app);
OLOLORD_EXPORT bool testAddFileParams(const AbstractBoard *board, cppcms::application &app,
                                      const Tools::PostParameters &params, const Tools::FileList &files,
                                      QString *error = 0);
OLOLORD_EXPORT bool testAddFileParamsAjax(const AbstractBoard *board, cppcms::application &app,
                                          const Tools::PostParameters &params, const Tools::FileList &files,
                                          QString *error = 0);
OLOLORD_EXPORT bool testAddFileParamsNonAjax(const AbstractBoard *board, cppcms::application &app,
                                             const Tools::PostParameters &params, const Tools::FileList &files,
                                             QString *error = 0);
OLOLORD_EXPORT bool testBan(cppcms::application &app, UserActionType proposedAction, const QString &board);
OLOLORD_EXPORT bool testBanAjax(cppcms::application &app, UserActionType proposedAction, const QString &board);
OLOLORD_EXPORT bool testBanNonAjax(cppcms::application &app, UserActionType proposedAction, const QString &board);
OLOLORD_EXPORT bool testParams(const AbstractBoard *board, cppcms::application &app,
                               const Tools::PostParameters &params, const Tools::FileList &files, bool post,
                               QString *error = 0);
OLOLORD_EXPORT bool testParamsAjax(const AbstractBoard *board, cppcms::application &app,
                                   const Tools::PostParameters &params, const Tools::FileList &files, bool post,
                                   QString *error = 0);
OLOLORD_EXPORT bool testParamsNonAjax(const AbstractBoard *board, cppcms::application &app,
                                      const Tools::PostParameters &params, const Tools::FileList &files, bool post,
                                      QString *error = 0);
OLOLORD_EXPORT bool testRequest(cppcms::application &app, int acceptedTypes, QString *error = 0);
OLOLORD_EXPORT bool testRequestAjax(cppcms::application &app, int acceptedTypes, QString *error = 0);
OLOLORD_EXPORT bool testRequestNonAjax(cppcms::application &app, int acceptedTypes, QString *error = 0);

}

#endif // CONTROLLER_H
