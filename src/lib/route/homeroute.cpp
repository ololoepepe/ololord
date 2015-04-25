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
    Tools::log(application, "home", "begin");
    QString err;
    if (!Controller::testRequest(application, Controller::GetRequest, &err))
        return Tools::log(application, "home", "fail:" + err);
    Content::Home c;
    TranslatorQt tq(application.request());
    TranslatorStd ts(application.request());
    Controller::initBase(c, application.request(),
                         tq.translate("HomeRoute", "ololord - (almost) free communication", "pageTitle"));
    c.custom = Tools::toStd(Tools::customHomePageContent(tq.locale()));
    foreach (const Tools::Friend &f, Tools::siteFriends()) {
        Content::Home::Friend ff;
        ff.name = Tools::toStd(f.name);
        ff.title = Tools::toStd(f.title);
        ff.url = Tools::toStd(f.url);
        c.friends.push_back(ff);
    }
    c.friendsHeader = ts.translate("HomeRoute", "Our friends", "friendsHeader");
    c.newsHeader = ts.translate("HomeRoute", "News", "newsHeader");
    foreach (const QString &s, Tools::news(tq.locale()))
        c.news.push_back(Tools::toStd(s));
    c.rulesHeader = ts.translate("HomeRoute", "Rules", "rulesHeader");
    foreach (const QString &s, Tools::rules("rules/home", tq.locale()))
        c.rules.push_back(Tools::toStd(s));
    c.welcomeMessage = ts.translate("HomeRoute", "Welcome. Again.", "welcomeMessage");
    application.render("home", c);
    Tools::log(application, "home", "success");
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
