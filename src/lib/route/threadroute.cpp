#include "threadroute.h"

#include "board/abstractboard.h"
#include "controller/controller.h"
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
    Tools::log(application, "Handling thread");
    if (!Controller::testRequest(application, Controller::GetRequest))
        return;
    AbstractBoard *board = AbstractBoard::board(Tools::fromStd(boardName));
    if (!board)
        return Controller::renderNotFound(application);
    board->handleThread(application, Tools::fromStd(thread).toULongLong());
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
    static const QString boardRx = "(" + AbstractBoard::boardNames().join("|") + ")";
    return Tools::toStd("/" + boardRx + "/thread/([1-9][0-9]*)\\.html");
}

std::string ThreadRoute::url() const
{
    return "/{1}/{2}";
}
