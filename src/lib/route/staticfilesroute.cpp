#include "staticfilesroute.h"

#include "board/abstractboard.h"
#include "cache.h"
#include "controller/controller.h"
#include "tools.h"

#include <BDirTools>
#include <BeQt>

#include <QByteArray>
#include <QDebug>
#include <QString>
#include <QStringList>

#include <cppcms/application.h>
#include <cppcms/http_request.h>
#include <cppcms/http_response.h>

StaticFilesRoute::StaticFilesRoute(cppcms::application &app, Mode m) :
    AbstractRoute(app), mode(m), Prefix((StaticFilesMode == m) ? "static" : "storage/img")
{
    //
}

void StaticFilesRoute::handle(std::string p)
{
    QString path = Tools::fromStd(p);
    QString logAction = QString(StaticFilesMode == mode ? "static" : "dynamic") + "_file";
    QString logTarget = path;
    Tools::log(application, logAction, "begin", logTarget);
    typedef QByteArray *(*GetCacheFunction)(const QString &path);
    typedef bool (*SetCacheFunction)(const QString &path, QByteArray *data);
    QString err;
    if (!Controller::testRequest(application, Controller::GetRequest, &err))
        return Tools::log(application, logAction, "fail:" + err, logTarget);
    if (path.contains("../") || path.contains("/..")) { //NOTE: Are you trying to cheat me?
        Controller::renderNotFound(application);
        Tools::log(application, logAction, "fail:cheating", logTarget);
        return;
    }
    GetCacheFunction getCache = (StaticFilesMode == mode) ? &Cache::staticFile : &Cache::dynamicFile;
    SetCacheFunction setCache = (StaticFilesMode == mode) ? &Cache::cacheStaticFile : &Cache::cacheDynamicFile;
    QByteArray *data = getCache(path);
    if (data) {
        application.response().content_type("");
        application.response().out().write(data->data(), data->size());
        Tools::log(application, logAction, "success:cache", logTarget);
    }
    QString fn = BDirTools::findResource(Prefix + "/" + path, BDirTools::AllResources);
    if (fn.startsWith(":")) { //NOTE: No need to cache files stored in memory
        bool ok = false;
        QByteArray ba = BDirTools::readFile(fn, -1, &ok);
        if (!ok) {
            Controller::renderNotFound(application);
            Tools::log(application, logAction, "fail:not_found", logTarget);
            return;
        }
        application.response().content_type("");
        application.response().out().write(ba.data(), ba.size());
        Tools::log(application, logAction, "success:in_memory", logTarget);
    }
    bool ok = false;
    data = new QByteArray(BDirTools::readFile(fn, -1, &ok));
    if (!ok) {
        Controller::renderNotFound(application);
        Tools::log(application, logAction, "fail:not_found", logTarget);
    }
    application.response().content_type("");
    application.response().out().write(data->data(), data->size());
    if (!setCache(path, data))
        delete data;
    Tools::log(application, logAction, "success", logTarget);
}

void StaticFilesRoute::handle(std::string boardName, std::string path)
{
    handle(boardName + "/" + path);
}

unsigned int StaticFilesRoute::handlerArgumentCount() const
{
    return (StaticFilesMode == mode) ? 1 : 2;
}

std::string StaticFilesRoute::key() const
{
    return (StaticFilesMode == mode) ? "staticFiles" : "dynamicFiles";
}

int StaticFilesRoute::priority() const
{
    return (StaticFilesMode == mode) ? 60 : 50;
}

std::string StaticFilesRoute::regex() const
{
    static const QString boardRx = "(" + AbstractBoard::boardNames().join("|") + ")";
    return (StaticFilesMode == mode) ? "/(.+)" : Tools::toStd("/" + boardRx + "/(.+)");
}

std::string StaticFilesRoute::url() const
{
    return (StaticFilesMode == mode) ? "/{1}" : "/{1}/{2}";
}
