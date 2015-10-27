#include "threadroute.h"

#include "board/abstractboard.h"
#include "controller.h"
#include "tools.h"

#include <QDebug>
#include <QString>
#include <QStringList>

#include <cppcms/application.h>
#include <cppcms/http_request.h>
#include <cppcms/http_response.h>

ThreadRoute::ThreadRoute(cppcms::application &app) :
    AbstractRoute(app)
{
    //
}

void ThreadRoute::handle(std::string boardName, std::string thread)
{
    DDOS_A(180)
    QString bn = Tools::fromStd(boardName);
    QString t = Tools::fromStd(thread);
    QString logTarget = bn + "/" + t;
    Tools::log(application, "thread", "begin", logTarget);
    QString err;
    if (!Controller::testRequestNonAjax(application, Controller::GetRequest, &err)) {
        Tools::log(application, "thread", "fail:" + err, logTarget);
        DDOS_POST_A
        return;
    }
    AbstractBoard::LockingWrapper board = AbstractBoard::board(bn);
    if (board.isNull()) {
        Controller::renderNotFoundNonAjax(application);
        Tools::log(application, "thread", "fail:not_found", logTarget);
        DDOS_POST_A
        return;
    }
    board->handleThread(application, t.toULongLong());
    DDOS_POST_A
}

unsigned int ThreadRoute::handlerArgumentCount() const
{
    return 2;
}

std::string ThreadRoute::key() const
{
    return "thread";
}

int ThreadRoute::priority() const
{
    return 10;
}

std::string ThreadRoute::regex() const
{
    QString boardRx = "(" + AbstractBoard::boardNames().join("|") + ")";
    return Tools::toStd("/" + boardRx + "/thread/([1-9][0-9]*)\\.html");
}

std::string ThreadRoute::url() const
{
    return "/{1}/{2}";
}
