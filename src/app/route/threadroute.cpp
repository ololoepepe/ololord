#include "threadroute.h"

#include <board/abstractboard.h>
#include <controller/controller.h>
#include <tools.h>

#include <QDebug>
#include <QString>

#include <cppcms/application.h>
#include <cppcms/http_request.h>
#include <cppcms/http_response.h>

ThreadRoute::ThreadRoute()
{
    //
}

void ThreadRoute::handle(cppcms::application &app, const QString &boardName, const QString &thread)
{
    Tools::log(app, "Handling thread");
    if (!Controller::testRequest(app, Controller::GetRequest))
        return;
    AbstractBoard *board = AbstractBoard::board(boardName);
    if (!board)
        return Controller::renderNotFound(app);
    board->handleThread(app, thread.toULongLong());
}
