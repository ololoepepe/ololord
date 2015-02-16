#include "boardroute.h"

#include "board/abstractboard.h"
#include "controller/controller.h"
#include "tools.h"

#include <QDebug>
#include <QString>
#include <QStringList>

#include <cppcms/application.h>
#include <cppcms/http_request.h>
#include <cppcms/http_response.h>

BoardRoute::BoardRoute(cppcms::application &app, Mode m) :
    AbstractRoute(app), mode(m)
{
    //
}

bool BoardRoute::duplicateWithSlashAppended() const
{
    return (BoardMode == mode);
}

void BoardRoute::handle(std::string boardName)
{
    if (BoardMode == mode) {
        Tools::log(application, "Handling board");
        if (!Controller::testRequest(application, Controller::GetRequest))
            return;
        AbstractBoard *board = AbstractBoard::board(Tools::fromStd(boardName));
        if (!board)
            return Controller::renderNotFound(application);
        board->handleBoard(application);
    } else if (BoardRulesRoute) {
        Tools::log(application, "Handling board rules");
        if (!Controller::testRequest(application, Controller::GetRequest))
            return;
        AbstractBoard *board = AbstractBoard::board(Tools::fromStd(boardName));
        if (!board)
            return Controller::renderNotFound(application);
        board->handleRules(application);
    }
}

void BoardRoute::handle(std::string boardName, std::string page)
{
    Tools::log(application, "Handling board page");
    if (!Controller::testRequest(application, Controller::GetRequest))
        return;
    AbstractBoard *board = AbstractBoard::board(Tools::fromStd(boardName));
    if (!board)
        return Controller::renderNotFound(application);
    board->handleBoard(application, Tools::fromStd(page).toUInt());
}

unsigned int BoardRoute::handlerArgumentCount() const
{
    switch (mode) {
    case BoardPageMode:
        return 2;
    case BoardMode:
    case BoardRulesRoute:
    default:
        return 1;
    }
}

std::string BoardRoute::key() const
{
    switch (mode) {
    case BoardPageMode:
        return "boardPage";
    case BoardRulesRoute:
        return "boardRules";
    case BoardMode:
    default:
        return "board";
    }
}

int BoardRoute::priority() const
{
    switch (mode) {
    case BoardPageMode:
        return 30;
    case BoardRulesRoute:
        return 20;
    case BoardMode:
    default:
        return 40;
    }
}

std::string BoardRoute::regex() const
{
    static const QString boardRx = "(" + AbstractBoard::boardNames().join("|") + ")";
    switch (mode) {
    case BoardPageMode:
        return Tools::toStd("/" + boardRx + "/([1-9][0-9]*)\\.html");
    case BoardRulesRoute:
        return Tools::toStd("/" + boardRx + "/rules\\.html");
    case BoardMode:
    default:
        return Tools::toStd("/" + boardRx);
    }
}

std::string BoardRoute::url() const
{
    switch (mode) {
    case BoardPageMode:
        return "/{1}/{2}";
    case BoardRulesRoute:
    case BoardMode:
    default:
        return "/{1}";
    }
}
