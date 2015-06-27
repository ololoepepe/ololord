#include "actionroute.h"

#include "board/abstractboard.h"
#include "controller/controller.h"
#include "database.h"
#include "settingslocker.h"
#include "tools.h"
#include "translator.h"

#include <BeQtGlobal>

#include <QByteArray>
#include <QCryptographicHash>
#include <QDebug>
#include <QRegExp>
#include <QSettings>
#include <QString>
#include <QStringList>
#include <QVariant>

#include <cppcms/application.h>
#include <cppcms/http_cookie.h>
#include <cppcms/http_request.h>
#include <cppcms/http_response.h>

#include <climits>
#include <string>

ActionRoute::ActionRoute(cppcms::application &app) :
    AbstractRoute(app)
{
    //
}

QStringList ActionRoute::availableActions()
{
    return actionMap().keys();
}

void ActionRoute::handle(std::string action)
{
    QString a = Tools::fromStd(action);
    Tools::PostParameters params = Tools::postParameters(application.request());
    QString logTarget = params.value("board");
    Tools::log(application, "action/" + a, "begin", logTarget);
    QString err;
    if (!Controller::testRequest(application, Controller::PostRequest, &err))
        return Tools::log(application, "action/" + a, "fail:" + err, logTarget);
    TranslatorQt tq(application.request());
    HandleActionMap map = actionMap();
    if (!map.contains(a)) {
        err = tq.translate("ActionRoute", "Unknown action", "error");
        Controller::renderError(application, err,
                                tq.translate("ActionRoute", "There is no such action", "description"));
        Tools::log(application, "action/" + a, "fail:" + err, logTarget);
        return;
    }
    (this->*map.value(a))(a, params, tq);
}

unsigned int ActionRoute::handlerArgumentCount() const
{
    return 1;
}

std::string ActionRoute::key() const
{
    return "action";
}

int ActionRoute::priority() const
{
    return 0;
}

std::string ActionRoute::regex() const
{
    static const QString actionRx = "(" + availableActions().join("|") + ")";
    return Tools::toStd("/action/" + actionRx);
}

std::string ActionRoute::url() const
{
    return "/action/{1}";
}

ActionRoute::HandleActionMap ActionRoute::actionMap()
{
    init_once(HandleActionMap, map, HandleActionMap()) {
        map.insert("add_file", &ActionRoute::handleAddFile);
        map.insert("ban_user", &ActionRoute::handleBanUser);
        map.insert("change_locale", &ActionRoute::handleChangeLocale);
        map.insert("change_settings", &ActionRoute::handleChangeSettings);
        map.insert("create_post", &ActionRoute::handleCreatePost);
        map.insert("create_thread", &ActionRoute::handleCreateThread);
        map.insert("delete_file", &ActionRoute::handleDeleteFile);
        map.insert("delete_post", &ActionRoute::handleDeletePost);
        map.insert("login", &ActionRoute::handleLogin);
        map.insert("logout", &ActionRoute::handleLogout);
    }
    return map;
}

void ActionRoute::handleAddFile(const QString &action, const Tools::PostParameters &params, const Translator::Qt &tq)
{
    AbstractBoard::LockingWrapper board = AbstractBoard::board(params.value("board"));
    if (!testBoard(board.data(), action, params.value("board"), tq))
        return;
    board->addFile(application);
}

void ActionRoute::handleBanUser(const QString &action, const Tools::PostParameters &params, const Translator::Qt &tq)
{
    QString sourceBoard = params.value("boardName");
    quint64 postNumber = params.value("postNumber").toULongLong();
    QString board = params.value("board");
    QString logTarget = sourceBoard + "/" + QString::number(postNumber);
    if (!Controller::testBanNonAjax(application, Controller::WriteAction, sourceBoard)
            || !Controller::testBanNonAjax(application, Controller::WriteAction, board)) {
        return Tools::log(application, "action/" + action, "fail:ban", logTarget);
    }
    QString reason = params.value("reason");
    int level = params.value("level").toInt();
    QDateTime expires = QDateTime::fromString(params.value("expires"), "dd.MM.yyyy:hh");
    QString err;
    if (!Database::banUser(application.request(), sourceBoard, postNumber, board, level, reason, expires, &err)) {
        Controller::renderErrorNonAjax(application, tq.translate("ActionRoute", "Failed to ban user", "error"), err);
        Tools::log(application, "action/" + action, "fail:" + err, logTarget);
        return;
    }
    redirect(sourceBoard + "/thread/" + QString::number(Database::postThreadNumber(sourceBoard, postNumber)) + ".html#"
             + QString::number(postNumber));
    Tools::log(application, "action/" + action, "success", logTarget);
}

void ActionRoute::handleChangeLocale(const QString &action, const Tools::PostParameters &params, const Translator::Qt &)
{
    setCookie("locale", "localeChangeSelect", params);
    redirect();
    Tools::log(application, "action/" + action, "success");
}

void ActionRoute::handleChangeSettings(const QString &action, const Tools::PostParameters &params,
                                       const Translator::Qt &)
{
    setCookie("mode", "modeChangeSelect", params);
    setCookie("style", "styleChangeSelect", params);
    setCookie("time", "timeChangeSelect", params);
    setCookie("captchaEngine", "captchaEngineSelect", params);
    redirect("settings");
    Tools::log(application, "action/" + action, "success");
}

void ActionRoute::handleCreatePost(const QString &action, const Tools::PostParameters &params,
                                   const Translator::Qt &tq)
{
    AbstractBoard::LockingWrapper board = AbstractBoard::board(params.value("board"));
    if (!testBoard(board.data(), action, params.value("board"), tq))
        return;
    board->createPost(application);
}

void ActionRoute::handleCreateThread(const QString &action, const Tools::PostParameters &params,
                                     const Translator::Qt &tq)
{
    AbstractBoard::LockingWrapper board = AbstractBoard::board(params.value("board"));
    if (!testBoard(board.data(), action, params.value("board"), tq))
        return;
    board->createThread(application);
}

void ActionRoute::handleDeleteFile(const QString &action, const Tools::PostParameters &params,
                                   const Translator::Qt &tq)
{
    QString boardName = params.value("boardName");
    quint64 postNumber = params.value("postNumber").toULongLong();
    QString fileName = params.value("fileName");
    QString logTarget = boardName + "/" + QString::number(postNumber) + "/" + fileName;
    if (!Controller::testBanNonAjax(application, Controller::WriteAction, boardName))
        return Tools::log(application, "action/" + action, "fail:ban", logTarget);
    QString err;
    if (!Database::deleteFile(boardName, fileName, application.request(), QByteArray(), &err)) {
        Controller::renderErrorNonAjax(application, tq.translate("ActionRoute", "Failed to delete file", "error"),
                                       err);
        Tools::log(application, "action/" + action, "fail:" + err, logTarget);
        return;
    }
    redirect(boardName + "/thread/" + QString::number(Database::postThreadNumber(boardName, postNumber)) + ".html#"
             + QString::number(postNumber));
    Tools::log(application, "action/" + action, "success", logTarget);
}

void ActionRoute::handleDeletePost(const QString &action, const Tools::PostParameters &params,
                                   const Translator::Qt &tq)
{
    QString boardName = params.value("boardName");
    quint64 postNumber = params.value("postNumber").toULongLong();
    QString logTarget = boardName + "/" + QString::number(postNumber);
    if (!Controller::testBanNonAjax(application, Controller::WriteAction, boardName))
        return Tools::log(application, "action/" + action, "fail:ban", logTarget);
    quint64 threadNumber = Database::postThreadNumber(boardName, postNumber);
    QString err;
    if (!Database::deletePost(boardName, postNumber, application.request(), QByteArray(), &err)) {
        Controller::renderErrorNonAjax(application, tq.translate("ActionRoute", "Failed to delete post", "error"),
                                       err);
        Tools::log(application, "action/" + action, "fail:" + err, logTarget);
        return;
    }
    QString path = boardName;
    if (threadNumber != postNumber)
        path += "/thread/" + QString::number(threadNumber) + ".html#" + QString::number(postNumber);
    redirect(path);
    Tools::log(application, "action/" + action, "success", logTarget);
}

void ActionRoute::handleLogin(const QString &action, const Tools::PostParameters &params, const Translator::Qt &)
{
    QString hashpass = params.value("hashpass");
    if (!QRegExp("").exactMatch(hashpass))
        hashpass = Tools::toString(QCryptographicHash::hash(hashpass.toUtf8(), QCryptographicHash::Sha1));
    application.response().set_cookie(cppcms::http::cookie("hashpass", Tools::toStd(hashpass), UINT_MAX, "/"));
    redirect();
    Tools::log(application, "action/" + action, "success");
}

void ActionRoute::handleLogout(const QString &action, const Tools::PostParameters &, const Translator::Qt &)
{
    application.response().set_cookie(cppcms::http::cookie("hashpass", "", UINT_MAX, "/"));
    redirect();
    Tools::log(application, "action/" + action, "success");
}

void ActionRoute::redirect(const QString &path)
{
    if (path.isEmpty()) {
        application.response().set_redirect_header(application.request().http_referer());
    } else {
        QString p = "/" + SettingsLocker()->value("Site/path_prefix").toString() + path;
        application.response().set_redirect_header(Tools::toStd(p));
    }
}

void ActionRoute::setCookie(const QString &name, const QString &sourceName, const Tools::PostParameters &params)
{
    if (name.isEmpty() || sourceName.isEmpty())
        return;
    QString value = params.value(sourceName);
    application.response().set_cookie(cppcms::http::cookie(Tools::toStd(name), Tools::toStd(value), UINT_MAX, "/"));
}

bool ActionRoute::testBoard(AbstractBoard *board, const QString &action, const QString &logTarget,
                            const Translator::Qt &tq)
{
    if (board)
        return true;
    QString err = tq.translate("ActionRoute", "Unknown board", "error");
    Controller::renderError(application, err, tq.translate("ActionRoute", "There is no such board", "description"));
    Tools::log(application, "action/" + action, "fail:" + err, logTarget);
    return false;
}
