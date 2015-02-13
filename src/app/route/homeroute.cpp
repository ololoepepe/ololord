#include "homeroute.h"

#include <controller/controller.h>
#include <controller/home.h>
#include <tools.h>
#include <translator.h>

#include <BDirTools>

#include <QCoreApplication>

#include <QDebug>
#include <QLocale>
#include <QSettings>
#include <QString>
#include <QStringList>
#include <QVariant>

#include <cppcms/application.h>
#include <cppcms/http_request.h>

HomeRoute::HomeRoute()
{
    //
}

void HomeRoute::handle(cppcms::application &app)
{
    Tools::log(app, "Handling home page");
    if (!Controller::testRequest(app, Controller::GetRequest))
        return;
    Content::Home c;
    TranslatorQt tq(app.request());
    TranslatorStd ts(app.request());
    Controller::initWithBase(&c, tq.locale());
    Controller::initWithNavbar(&c, tq.locale());
    Controller::initWithSettings(&c, tq.locale());
    c.pageTitle = ts.translate("HomeRoute", "ololord - (almost) free communication", "pageTitle");
    foreach (const QString &s, Tools::rules("rules/home", tq.locale()))
        c.rules.push_back(Tools::toStd(Controller::toHtml(s)));
    c.welcomeMessage = ts.translate("HomeRoute", "Welcome. Again.", "welcomeMessage");
    app.render("home", c);
    Tools::log(app, "Handled home page successfully");
}
