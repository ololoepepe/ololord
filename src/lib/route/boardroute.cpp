#include "boardroute.h"

#include "board/abstractboard.h"
#include "controller.h"
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
    QString bn = Tools::fromStd(boardName);
    QString logTarget = bn;
    QString err;
    if (BoardMode == mode) {
        logTarget += "/0";
        Tools::log(application, "board", "begin", logTarget);
        if (!Controller::testRequestNonAjax(application, Controller::GetRequest, &err))
            return Tools::log(application, "board", "fail:" + err, logTarget);
        AbstractBoard::LockingWrapper board = AbstractBoard::board(bn);
        if (board.isNull()) {
            Controller::renderNotFoundNonAjax(application);
            Tools::log(application, "board", "fail:not_found", logTarget);
            return;
        }
        board->handleBoard(application);
    } else if (BoardRulesRoute) {
        Tools::log(application, "board_rules", "begin", logTarget);
        if (!Controller::testRequestNonAjax(application, Controller::GetRequest, &err))
            return Tools::log(application, "board", "fail:" + err, logTarget);
        AbstractBoard::LockingWrapper board = AbstractBoard::board(bn);
        if (board.isNull()) {
            Controller::renderNotFoundNonAjax(application);
            Tools::log(application, "board", "fail:not_found", logTarget);
            return;
        }
        board->handleRules(application);
    }
}

void BoardRoute::handle(std::string boardName, std::string page)
{
    QString bn = Tools::fromStd(boardName);
    QString p = Tools::fromStd(page);
    QString logTarget = bn + "/" + p;
    Tools::log(application, "board", "begin", logTarget);
    QString err;
    if (!Controller::testRequestNonAjax(application, Controller::GetRequest, &err))
        return Tools::log(application, "board", "fail:" + err, logTarget);
    AbstractBoard::LockingWrapper board = AbstractBoard::board(bn);
    if (board.isNull()) {
        Tools::log(application, "board", "fail:not_found", logTarget);
        Controller::renderNotFoundNonAjax(application);
        return;
    }
    board->handleBoard(application, p.toUInt());
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
    QString boardRx = "(" + AbstractBoard::boardNames().join("|") + ")";
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
