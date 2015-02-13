#include "boardroute.h"

#include <board/abstractboard.h>
#include <controller/controller.h>
#include <tools.h>

#include <QDebug>
#include <QString>

#include <cppcms/application.h>
#include <cppcms/http_request.h>
#include <cppcms/http_response.h>

BoardRoute::BoardRoute()
{
    //
}

void BoardRoute::handle(cppcms::application &app, const QString &boardName)
{
    Tools::log(app, "Handling board");
    if (!Controller::testRequest(app, Controller::GetRequest))
        return;
    AbstractBoard *board = AbstractBoard::board(boardName);
    if (!board)
        return Controller::renderNotFound(app);
    board->handleBoard(app);
}

void BoardRoute::handle(cppcms::application &app, const QString &boardName, const QString &page)
{
    Tools::log(app, "Handling board page");
    if (!Controller::testRequest(app, Controller::GetRequest))
        return;
    AbstractBoard *board = AbstractBoard::board(boardName);
    if (!board)
        return Controller::renderNotFound(app);
    board->handleBoard(app, page.toUInt());
}

void BoardRoute::handleRules(cppcms::application &app, const QString &boardName)
{
    Tools::log(app, "Handling board rules");
    if (!Controller::testRequest(app, Controller::GetRequest))
        return;
    AbstractBoard *board = AbstractBoard::board(boardName);
    if (!board)
        return Controller::renderNotFound(app);
    board->handleRules(app);
}
