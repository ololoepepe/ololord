#include "settingsroute.h"

#include "controller/controller.h"
#include "controller/settings.h"
#include "database.h"
#include "search.h"
#include "stored/thread.h"
#include "stored/thread-odb.hxx"
#include "tools.h"
#include "transaction.h"
#include "translator.h"

#include <BTextTools>

#include <QDebug>
#include <QList>
#include <QRegExp>
#include <QString>
#include <QStringList>

#include <cppcms/application.h>
#include <cppcms/http_request.h>

SettingsRoute::SettingsRoute(cppcms::application &app) :
    AbstractRoute(app)
{
    //
}

void SettingsRoute::handle()
{
    Tools::log(application, "settings", "begin");
    QString err;
    if (!Controller::testRequest(application, Controller::GetRequest, &err))
        return Tools::log(application, "settings", "fail:" + err);
    Content::Settings c;
    TranslatorQt tq(application.request());
    Controller::initBase(c, application.request(), tq.translate("SettingsRoute", "Settings", "pageTitle"));
    Tools::render(application, "settings_view", c);
    Tools::log(application, "settings", "success");
}

unsigned int SettingsRoute::handlerArgumentCount() const
{
    return 0;
}

std::string SettingsRoute::key() const
{
    return "settings";
}

int SettingsRoute::priority() const
{
    return 7;
}

std::string SettingsRoute::regex() const
{
    return "/settings";
}

std::string SettingsRoute::url() const
{
    return "/settings";
}
