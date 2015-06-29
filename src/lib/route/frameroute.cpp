#include "frameroute.h"

#include "controller/controller.h"
#include "controller/frame.h"
#include "tools.h"
#include "translator.h"

#include <BeQtGlobal>

#include <QDebug>
#include <QString>
#include <QStringList>

#include <cppcms/application.h>
#include <cppcms/http_request.h>

FrameRoute::FrameRoute(cppcms::application &app) :
    AbstractRoute(app)
{
    //
}

void FrameRoute::handle()
{
    QString path = Tools::getParameters(application.request()).value("path");
    QString logTarget = path;
    Tools::log(application, "frame", "begin", logTarget);
    QString err;
    if (!Controller::testRequestNonAjax(application, Controller::GetRequest, &err))
        return Tools::log(application, "frame", "fail:" + err, logTarget);
    Content::Frame c;
    TranslatorQt tq(application.request());
    Controller::initBase(c, application.request(),
                         tq.translate("FrameRoute", "ololord - (almost) free communication", "pageTitle"));
    c.sourcePath = Tools::toStd(path);
    Tools::render(application, "frame", c);
    Tools::log(application, "frame", "success", logTarget);
}

unsigned int FrameRoute::handlerArgumentCount() const
{
    return 0;
}

std::string FrameRoute::key() const
{
    return "frame";
}

int FrameRoute::priority() const
{
    return 0;
}

std::string FrameRoute::regex() const
{
    return "/frame";
}

std::string FrameRoute::url() const
{
    return "/frame";
}
