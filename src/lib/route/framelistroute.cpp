#include "framelistroute.h"

#include "controller.h"
#include "controller/framelist.h"
#include "tools.h"
#include "translator.h"

#include <BeQtGlobal>

#include <QDebug>
#include <QString>
#include <QStringList>

#include <cppcms/application.h>
#include <cppcms/http_request.h>

FrameListRoute::FrameListRoute(cppcms::application &app) :
    AbstractRoute(app)
{
    //
}

void FrameListRoute::handle()
{
    Tools::log(application, "frame_list", "begin");
    QString err;
    if (!Controller::testRequestNonAjax(application, Controller::GetRequest, &err))
        return Tools::log(application, "frame_list", "fail:" + err);
    Content::FrameList c;
    TranslatorQt tq(application.request());
    TranslatorStd ts(tq.locale());
    Controller::initBase(c, application.request(),
                         tq.translate("FrameListRoute", "ololord - (almost) free communication", "pageTitle"));
    c.normalVersionText = ts.translate("FrameListRoute", "Version without frame", "normalVersionText");
    Tools::render(application, "frame_list", c);
    Tools::log(application, "frame_list", "success");
}

unsigned int FrameListRoute::handlerArgumentCount() const
{
    return 0;
}

std::string FrameListRoute::key() const
{
    return "frame_list";
}

int FrameListRoute::priority() const
{
    return 0;
}

std::string FrameListRoute::regex() const
{
    return "/frame_list";
}

std::string FrameListRoute::url() const
{
    return "/frame_list";
}
