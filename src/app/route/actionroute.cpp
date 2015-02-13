#include "actionroute.h"

#include <board/abstractboard.h>
#include <controller/controller.h>
#include <tools.h>
#include <translator.h>

#include <BeQtGlobal>

#include <QDebug>
#include <QString>
#include <QStringList>

#include <cppcms/application.h>
#include <cppcms/http_request.h>
#include <cppcms/http_response.h>

#include <string>

ActionRoute::ActionRoute()
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

void ActionRoute::handle(cppcms::application &app, const QString &action)
{
    Tools::log(app, "Handling action");
    if (!Controller::testRequest(app, Controller::PostRequest))
        return;
    TranslatorQt tq(app.request());
    if (!availableActions().contains(action)) {
        return Controller::renderError(app, tq.translate("ActionRoute", "Unknown action", "error"),
                                       tq.translate("ActionRoute", "There is no such action", "description"));
    }
    QString boardName = Tools::postParameters(app.request()).value("board");
    AbstractBoard *board = AbstractBoard::board(boardName);
    if (!board) {
        return Controller::renderError(app, tq.translate("ActionRoute", "Unknown board", "error"),
                                       tq.translate("ActionRoute", "There is no such board", "description"));
    }
    if ("create_post" == action) {
        board->createPost(app);
    } else if ("create_thread" == action) {
        board->createThread(app);
    } else {
        Controller::renderError(app, tq.translate("ActionRoute", "Unknown action", "error"),
                                tq.translate("ActionRoute", "There is no such action", "description"));
    }
}
