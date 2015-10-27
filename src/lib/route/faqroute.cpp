#include "faqroute.h"

#include "controller.h"
#include "controller/faq.h"
#include "tools.h"
#include "translator.h"

#include <QDebug>
#include <QLocale>
#include <QString>
#include <QStringList>

#include <cppcms/application.h>
#include <cppcms/http_request.h>

FaqRoute::FaqRoute(cppcms::application &app) :
    AbstractRoute(app)
{
    //
}

void FaqRoute::handle()
{
    DDOS_A(16)
    Tools::log(application, "faq", "begin");
    QString err;
    if (!Controller::testRequestNonAjax(application, Controller::GetRequest, &err)) {
        Tools::log(application, "faq", "fail:" + err);
        DDOS_POST_A
        return;
    }
    Content::Faq c;
    TranslatorQt tq(application.request());
    Controller::initBase(c, application.request(), tq.translate("FaqRoute", "F.A.Q.", "pageTitle"));
    QString deviceType = Tools::isMobile(application.request()).any ? "mobile" : "desktop";
    c.custom = Tools::toStd(Tools::customContent("faq", tq.locale()).replace("%deviceType%", deviceType));
    Tools::render(application, "faq", c);
    Tools::log(application, "faq", "success");
    DDOS_POST_A
}

unsigned int FaqRoute::handlerArgumentCount() const
{
    return 0;
}

std::string FaqRoute::key() const
{
    return "faq";
}

int FaqRoute::priority() const
{
    return 0;
}

std::string FaqRoute::regex() const
{
    return "/faq";
}

std::string FaqRoute::url() const
{
    return "/faq";
}
