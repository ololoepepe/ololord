#ifndef CONTROLLER_H
#define CONTROLLER_H

class AbstractBoard;
class HelperPost;
class Post;
class WithBanner;
class WithBase;
class WithNavbar;
class WithPostForm;
class WithPosts;
class WithSettings;

class QLocale;

namespace cppcms
{

class application;

}

#include "../global.h"
#include "tools.h"

#include <QDateTime>
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

void OLOLORD_EXPORT initWithBanner(WithBanner *w, const QLocale &l, const AbstractBoard *board);
void OLOLORD_EXPORT initWithBase(WithBase *w, const QLocale &l);
void OLOLORD_EXPORT initWithNavbar(WithNavbar *w, const QLocale &l);
void OLOLORD_EXPORT initWithPostForm(WithPostForm *w, const QLocale &l, const AbstractBoard *board);
void OLOLORD_EXPORT initWithPosts(WithPosts *w, const QLocale &l);
void OLOLORD_EXPORT initWithSettings(WithSettings *w, const QLocale &l);
void OLOLORD_EXPORT redirect(cppcms::application &app, const QString &where);
void OLOLORD_EXPORT renderBan(cppcms::application &app, const QString &board, int level, const QDateTime &dateTime,
                              const QString &reason = QString(), const QDateTime &expires = QDateTime());
void OLOLORD_EXPORT renderError(cppcms::application &app, const QString &error,
                                const QString &description = QString());
void OLOLORD_EXPORT renderNotFound(cppcms::application &app);
bool OLOLORD_EXPORT testBan(cppcms::application &app, UserActionType proposedAction, const QString &board);
bool OLOLORD_EXPORT testParams(cppcms::application &app, const Tools::PostParameters &params);
bool OLOLORD_EXPORT testRequest(cppcms::application &app, int acceptedTypes);
QString OLOLORD_EXPORT toHtml(const QString &s);
void OLOLORD_EXPORT toHtml(QString *s);
HelperPost OLOLORD_EXPORT toController(const Post &post, const QString &boardName, quint64 opPostId, const QLocale &l,
                                       bool processCode = false);

}

#endif // CONTROLLER_H
