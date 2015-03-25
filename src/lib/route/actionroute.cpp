#include "actionroute.h"

#include "board/abstractboard.h"
#include "controller/controller.h"
#include "tools.h"
#include "translator.h"

#include <BeQtGlobal>

#include <QDebug>
#include <QString>
#include <QStringList>

#include <cppcms/application.h>
#include <cppcms/http_request.h>
#include <cppcms/http_response.h>

#include <string>

ActionRoute::ActionRoute(cppcms::application &app) :
    AbstractRoute(app)
{
    //
}

QStringList ActionRoute::availableActions()
{
    init_once(QStringList, actions, QStringList()) {
        actions << "create_post";
        actions << "create_thread";
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
    AbstractBoard *board = AbstractBoard::board(boardName);
    if (!board) {
        err = tq.translate("ActionRoute", "Unknown board", "error");
        Controller::renderError(application, err,
                                tq.translate("ActionRoute", "There is no such board", "description"));
        Tools::log(application, a, "fail:" + err, logTarget);
        return;
    }
    if ("create_post" == action) {
        board->createPost(application);
    } else if ("create_thread" == action) {
        board->createThread(application);
    } else {
        err = tq.translate("ActionRoute", "Unknown action", "error");
        Controller::renderError(application, err,
                                tq.translate("ActionRoute", "There is no such action", "description"));
        Tools::log(application, a, "fail:" + err, logTarget);
        return;
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
