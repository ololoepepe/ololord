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
    init_once(QStringList, actions, QStringList()) {
        actions << "add_file";
        actions << "change_locale";
        actions << "change_settings";
        actions << "create_post";
        actions << "create_thread";
        actions << "login";
        actions << "logout";
    }
    return actions;
}

void ActionRoute::handle(std::string action)
{
    QString a = Tools::fromStd(action);
    QString boardName = Tools::postParameters(application.request()).value("board");
    QString logTarget = boardName;
    Tools::log(application, a, "begin", logTarget);
    QString err;
    if (!Controller::testRequest(application, Controller::PostRequest, &err))
        return Tools::log(application, a, "fail:" + err, logTarget);
    TranslatorQt tq(application.request());
    if (!availableActions().contains(a)) {
        err = tq.translate("ActionRoute", "Unknown action", "error");
        Controller::renderError(application, err,
                                tq.translate("ActionRoute", "There is no such action", "description"));
        Tools::log(application, a, "fail:" + err, logTarget);
        return;
    }
    if ("create_post" == action) {
        AbstractBoard::LockingWrapper board = AbstractBoard::board(boardName);
        if (!testBoard(board.data(), a, logTarget, tq))
            return;
        board->createPost(application);
    } else if ("create_thread" == action) {
        AbstractBoard::LockingWrapper board = AbstractBoard::board(boardName);
        if (!testBoard(board.data(), a, logTarget, tq))
            return;
        board->createThread(application);
    } else if ("add_file" == action) {
        AbstractBoard::LockingWrapper board = AbstractBoard::board(boardName);
        if (!testBoard(board.data(), a, logTarget, tq))
            return;
        board->addFile(application);
    } else if ("change_settings" == action) {
        setCookie("mode", "modeChangeSelect");
        setCookie("style", "styleChangeSelect");
        setCookie("time", "timeChangeSelect");
        setCookie("captchaEngine", "captchaEngineSelect");
        redirect();
    } else if ("change_locale" == action) {
        setCookie("locale", "localeChangeSelect");
        redirect();
    } else if ("login" == action) {
        QString hashpass = Tools::postParameters(application.request()).value("hashpass");
        if (!QRegExp("").exactMatch(hashpass))
            hashpass = Tools::toString(QCryptographicHash::hash(hashpass.toUtf8(), QCryptographicHash::Sha1));
        application.response().set_cookie(cppcms::http::cookie("hashpass", Tools::toStd(hashpass), UINT_MAX, "/"));
        redirect();
    } else if ("logout" == action) {
        application.response().set_cookie(cppcms::http::cookie("hashpass", "", UINT_MAX, "/"));
        redirect();
    } else {
        err = tq.translate("ActionRoute", "Unknown action", "error");
        Controller::renderError(application, err,
                                tq.translate("ActionRoute", "There is no such action", "description"));
        Tools::log(application, a, "fail:" + err, logTarget);
    }
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

void ActionRoute::redirect(const QString &path)
{
    QString p = "/" + SettingsLocker()->value("Site/path_prefix").toString() + path;
    application.response().set_redirect_header(Tools::toStd(p));
}

void ActionRoute::setCookie(const QString &name, const QString &sourceName)
{
    if (name.isEmpty() || sourceName.isEmpty())
        return;
    QString value = Tools::postParameters(application.request()).value(sourceName);
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
