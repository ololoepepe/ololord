#include "actionroute.h"

#include "board/abstractboard.h"
#include "controller/controller.h"
#include "settingslocker.h"
#include "tools.h"
#include "translator.h"

#include <BeQtGlobal>

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
    Tools::log(application, a, "begin", logTarget);
    QString err;
    if (!Controller::testRequest(application, Controller::PostRequest, &err))
        return Tools::log(application, a, "fail:" + err, logTarget);
    TranslatorQt tq(application.request());
    HandleActionMap map = actionMap();
    if (!map.contains(a)) {
        err = tq.translate("ActionRoute", "Unknown action", "error");
        Controller::renderError(application, err,
                                tq.translate("ActionRoute", "There is no such action", "description"));
        Tools::log(application, a, "fail:" + err, logTarget);
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
        map.insert("change_locale", &ActionRoute::handleChangeLocale);
        map.insert("change_settings", &ActionRoute::handleChangeSettings);
        map.insert("create_post", &ActionRoute::handleCreatePost);
        map.insert("create_thread", &ActionRoute::handleCreateThread);
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

void ActionRoute::handleChangeLocale(const QString &, const Tools::PostParameters &params, const Translator::Qt &)
{
    setCookie("locale", "localeChangeSelect", params);
    redirect();
}

void ActionRoute::handleChangeSettings(const QString &, const Tools::PostParameters &params, const Translator::Qt &)
{
    setCookie("mode", "modeChangeSelect", params);
    setCookie("style", "styleChangeSelect", params);
    setCookie("time", "timeChangeSelect", params);
    setCookie("captchaEngine", "captchaEngineSelect", params);
    redirect();
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

void ActionRoute::handleLogin(const QString &, const Tools::PostParameters &params, const Translator::Qt &)
{
    QString hashpass = params.value("hashpass");
    if (!QRegExp("").exactMatch(hashpass))
        hashpass = Tools::toString(QCryptographicHash::hash(hashpass.toUtf8(), QCryptographicHash::Sha1));
    application.response().set_cookie(cppcms::http::cookie("hashpass", Tools::toStd(hashpass), UINT_MAX, "/"));
    redirect();
}

void ActionRoute::handleLogout(const QString &, const Tools::PostParameters &, const Translator::Qt &)
{
    application.response().set_cookie(cppcms::http::cookie("hashpass", "", UINT_MAX, "/"));
    redirect();
}

void ActionRoute::redirect(const QString &path)
{
    QString p = "/" + SettingsLocker()->value("Site/path_prefix").toString() + path;
    application.response().set_redirect_header(Tools::toStd(p));
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
    Tools::log(application, action, "fail:" + err, logTarget);
    return false;
}
