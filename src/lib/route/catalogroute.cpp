#include "catalogroute.h"

#include "board/abstractboard.h"
#include "controller/controller.h"
#include "tools.h"

#include <QDebug>
#include <QString>
#include <QStringList>

#include <cppcms/application.h>
#include <cppcms/http_request.h>
#include <cppcms/http_response.h>

CatalogRoute::CatalogRoute(cppcms::application &app) :
    AbstractRoute(app)
{
    //
}

void CatalogRoute::handle(std::string boardName)
{
    QString bn = Tools::fromStd(boardName);
    QString logTarget = bn;
    Tools::log(application, "catalog", "begin", logTarget);
    QString err;
    if (!Controller::testRequestNonAjax(application, Controller::GetRequest, &err))
        return Tools::log(application, "catalog", "fail:" + err, logTarget);
    AbstractBoard::LockingWrapper board = AbstractBoard::board(bn);
    if (board.isNull()) {
        Controller::renderNotFoundNonAjax(application);
        Tools::log(application, "catalog", "fail:not_found", logTarget);
        return;
    }
    board->handleCatalog(application);
}

unsigned int CatalogRoute::handlerArgumentCount() const
{
    return 1;
}

std::string CatalogRoute::key() const
{
    return "catalog";
}

int CatalogRoute::priority() const
{
    return 5;
}

std::string CatalogRoute::regex() const
{
    QString boardRx = "(" + AbstractBoard::boardNames().join("|") + ")";
    return Tools::toStd("/" + boardRx + "/catalog\\.html");
}

std::string CatalogRoute::url() const
{
    return "/{1}";
}
