#include "homeroute.h"

#include "controller/controller.h"
#include "controller/home.h"
#include "tools.h"
#include "translator.h"

#include <QDebug>
#include <QLocale>
#include <QString>
#include <QStringList>

#include <cppcms/application.h>
#include <cppcms/http_request.h>

HomeRoute::HomeRoute(cppcms::application &app) :
    AbstractRoute(app)
{
    //
}

void HomeRoute::handle()
{
    Tools::log(application, "Handling home page");
    if (!Controller::testRequest(application, Controller::GetRequest))
        return;
    Content::Home c;
    TranslatorQt tq(application.request());
    TranslatorStd ts(application.request());
    Controller::initBase(c, application.request(),
                         tq.translate("HomeRoute", "ololord - (almost) free communication", "pageTitle"));
    c.newsHeader = ts.translate("HomeRoute", "News", "newsHeader");
    foreach (const QString &s, Tools::news(tq.locale()))
        c.news.push_back(Tools::toStd(s));
    c.rulesHeader = ts.translate("HomeRoute", "Rules", "rulesHeader");
    foreach (const QString &s, Tools::rules("rules/home", tq.locale()))
        c.rules.push_back(Tools::toStd(s));
    c.welcomeMessage = ts.translate("HomeRoute", "Welcome. Again.", "welcomeMessage");
    application.render("home", c);
    Tools::log(application, "Handled home page successfully");
}

unsigned int HomeRoute::handlerArgumentCount() const
{
    return 0;
}

std::string HomeRoute::key() const
{
    return "";
}

int HomeRoute::priority() const
{
    return 70;
}

std::string HomeRoute::regex() const
{
    return "/";
}

std::string HomeRoute::url() const
{
    return "";
}
