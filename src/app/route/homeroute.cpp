#include "homeroute.h"

#include <controller/controller.h>
#include <controller/home.h>
#include <tools.h>
#include <translator.h>

#include <QDebug>
#include <QLocale>
#include <QString>
#include <QStringList>

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
    Controller::initBase(c, tq.locale(),
                         tq.translate("HomeRoute", "ololord - (almost) free communication", "pageTitle"));
    foreach (const QString &s, Tools::rules("rules/home", tq.locale()))
        c.rules.push_back(Tools::toStd(Controller::toHtml(s)));
    c.welcomeMessage = ts.translate("HomeRoute", "Welcome. Again.", "welcomeMessage");
    app.render("home", c);
    Tools::log(app, "Handled home page successfully");
}
