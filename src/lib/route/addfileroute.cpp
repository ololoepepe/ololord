#include "addfileroute.h"

#include "controller/controller.h"
#include "controller/addfile.h"
#include "database.h"
#include "tools.h"
#include "translator.h"

#include <BeQtGlobal>

#include <QDebug>
#include <QString>
#include <QStringList>

#include <cppcms/application.h>
#include <cppcms/http_request.h>

AddFileRoute::AddFileRoute(cppcms::application &app) :
    AbstractRoute(app)
{
    //
}

void AddFileRoute::handle()
{
    Tools::GetParameters params = Tools::getParameters(application.request());
    QString boardName = params.value("board");
    quint64 postNumber = params.value("post").toULongLong();
    QString logTarget = boardName + "/" + QString::number(postNumber);
    Tools::log(application, "add_file", "begin", logTarget);
    QString err;
    if (!Controller::testRequestNonAjax(application, Controller::GetRequest, &err))
        return Tools::log(application, "add_file", "fail:" + err, logTarget);
    TranslatorQt tq(application.request());
    if (Tools::hashpassString(application.request()).isEmpty()) {
        QString err = tq.translate("AddFileRoute", "Access error", "error");
        Controller::renderErrorNonAjax(application, err,
                                       tq.translate("AddFileRoute", "Not enough rights", "description"));
        Tools::log(application, "add_file", "fail:" + err, logTarget);
        return;
    }
    if (boardName.isEmpty()) {
        QString err = tq.translate("AddFileRoute", "Invalid board name", "error");
        Controller::renderErrorNonAjax(application, err,
                                       tq.translate("AddFileRoute", "Board name is empty", "description"));
        Tools::log(application, "add_file", "fail:" + err, logTarget);
        return;
    }
    if (!postNumber) {
        QString err = tq.translate("AddFileRoute", "Invalid post number", "error");
        Controller::renderErrorNonAjax(application, err,
                                       tq.translate("AddFileRoute", "Post number is null", "description"));
        Tools::log(application, "add_file", "fail:" + err, logTarget);
        return;
    }
    AbstractBoard::LockingWrapper board = AbstractBoard::board(boardName);
    if (board.isNull()) {
        QString err = tq.translate("AddFileRoute", "Unknown board", "error");
        Controller::renderErrorNonAjax(application, err,
                                       tq.translate("AddFileRoute", "There is no such board", "description"));
        Tools::log(application, "add_file", "fail:" + err, logTarget);
        return;
    }
    Content::AddFile c;
    Controller::initBase(c, application.request(), tq.translate("AddFileRoute", "Add file", "pageTitle"));
    c.currentBoardName = Tools::toStd(boardName);
    c.maxFileCount = Tools::maxInfo(Tools::MaxFileCount, boardName) - Database::fileCount(boardName, postNumber);
    c.postNumber = postNumber;
    c.supportedFileTypes = Tools::toStd(board->supportedFileTypes());
    Tools::render(application, "add_file", c);
    Tools::log(application, "add_file", "success", logTarget);
}

unsigned int AddFileRoute::handlerArgumentCount() const
{
    return 0;
}

std::string AddFileRoute::key() const
{
    return "add_file";
}

int AddFileRoute::priority() const
{
    return 0;
}

std::string AddFileRoute::regex() const
{
    return "/add_file";
}

std::string AddFileRoute::url() const
{
    return "/add_file";
}
