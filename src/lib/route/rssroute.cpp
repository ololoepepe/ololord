#include "rssroute.h"

#include "board/abstractboard.h"
#include "controller.h"
#include "database.h"
#include "tools.h"

#include <QDebug>
#include <QString>
#include <QStringList>

#include <cppcms/application.h>
#include <cppcms/http_request.h>
#include <cppcms/http_response.h>

RssRoute::RssRoute(cppcms::application &app) :
    AbstractRoute(app)
{
    //
}

void RssRoute::handle(std::string boardName)
{
    QString bn = Tools::fromStd(boardName);
    QString logTarget = bn;
    Tools::log(application, "rss", "begin", logTarget);
    QString err;
    if (!Controller::testRequestNonAjax(application, Controller::GetRequest, &err))
        return Tools::log(application, "rss", "fail:" + err, logTarget);
    QString rss = Database::rss(bn);
    if (rss.isEmpty()) {
        Controller::renderNotFoundNonAjax(application);
        Tools::log(application, "rss", "fail:not_found", logTarget);
        return;
    }
    application.response().out() << Tools::toStd(rss);
}

unsigned int RssRoute::handlerArgumentCount() const
{
    return 1;
}

std::string RssRoute::key() const
{
    return "rss";
}

int RssRoute::priority() const
{
    return 5;
}

std::string RssRoute::regex() const
{
    QString boardRx = "(" + AbstractBoard::boardNames().join("|") + ")";
    return Tools::toStd("/" + boardRx + "/rss\\.xml");
}

std::string RssRoute::url() const
{
    return "/{1}";
}
