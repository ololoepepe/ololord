#include "movethreadroute.h"

#include "controller.h"
#include "controller/movethread.h"
#include "database.h"
#include "tools.h"
#include "translator.h"

#include <BeQtGlobal>

#include <QDebug>
#include <QString>
#include <QStringList>

#include <cppcms/application.h>
#include <cppcms/http_request.h>

MoveThreadRoute::MoveThreadRoute(cppcms::application &app) :
    AbstractRoute(app)
{
    //
}

void MoveThreadRoute::handle()
{
    DDOS_A(12)
    Tools::GetParameters params = Tools::getParameters(application.request());
    QString boardName = params.value("board");
    quint64 threadNumber = params.value("thread").toULongLong();
    QString logTarget = boardName + "/" + QString::number(threadNumber);
    Tools::log(application, "move_thread", "begin", logTarget);
    QString err;
    if (!Controller::testRequestNonAjax(application, Controller::GetRequest, &err)) {
        Tools::log(application, "move_thread", "fail:" + err, logTarget);
        DDOS_POST_A
        return;
    }
    TranslatorQt tq(application.request());
    if (Tools::hashpassString(application.request()).isEmpty()) {
        QString err = tq.translate("MoveThreadRoute", "Access error", "error");
        Controller::renderErrorNonAjax(application, err,
                                       tq.translate("MoveThreadRoute", "Not enough rights", "description"));
        Tools::log(application, "move_thread", "fail:" + err, logTarget);
        DDOS_POST_A
        return;
    }
    if (boardName.isEmpty()) {
        QString err = tq.translate("MoveThreadRoute", "Invalid board name", "error");
        Controller::renderErrorNonAjax(application, err,
                                       tq.translate("MoveThreadRoute", "Board name is empty", "description"));
        Tools::log(application, "move_thread", "fail:" + err, logTarget);
        DDOS_POST_A
        return;
    }
    if (!threadNumber) {
        QString err = tq.translate("MoveThreadRoute", "Invalid thread number", "error");
        Controller::renderErrorNonAjax(application, err,
                                       tq.translate("MoveThreadRoute", "Thread number is null", "description"));
        Tools::log(application, "move_thread", "fail:" + err, logTarget);
        DDOS_POST_A
        return;
    }
    AbstractBoard::LockingWrapper board = AbstractBoard::board(boardName);
    if (board.isNull()) {
        QString err = tq.translate("MoveThreadRoute", "Unknown board", "error");
        Controller::renderErrorNonAjax(application, err,
                                       tq.translate("MoveThreadRoute", "There is no such board", "description"));
        Tools::log(application, "thread_number", "fail:" + err, logTarget);
        DDOS_POST_A
        return;
    }
    Content::MoveThread c;
    Controller::initBase(c, application.request(), tq.translate("MoveThreadRoute", "Move thread", "pageTitle"));
    TranslatorStd ts(application.request());
    c.currentBoardName = Tools::toStd(boardName);
    c.moveThreadWarningText = ts.translate("MoveThreadRoute", "Warning: post numbers will be changed, and so will the "
                                           "post references. But the raw post text will not bechanged, so be careful "
                                           "when editing posts in moved thread.", "moveThreadWarningText");
    c.threadNumber = threadNumber;
    QStringList userBoards = Database::registeredUserBoards(application.request());
    if (userBoards.size() == 1 && userBoards.first() == "*")
        userBoards << AbstractBoard::boardNames();
    userBoards.removeAll("*");
    foreach (const QString &s, userBoards) {
        AbstractBoard::BoardInfo inf;
        inf.name = Tools::toStd(s);
        AbstractBoard::LockingWrapper b = AbstractBoard::board(s);
        inf.title = Tools::toStd(b->title(tq.locale()));
        c.availableBoards.push_back(inf);
    }
    Tools::render(application, "move_thread", c);
    Tools::log(application, "move_thread", "success", logTarget);
    DDOS_POST_A
}

unsigned int MoveThreadRoute::handlerArgumentCount() const
{
    return 0;
}

std::string MoveThreadRoute::key() const
{
    return "move_thread";
}

int MoveThreadRoute::priority() const
{
    return 0;
}

std::string MoveThreadRoute::regex() const
{
    return "/move_thread";
}

std::string MoveThreadRoute::url() const
{
    return "/move_thread";
}
