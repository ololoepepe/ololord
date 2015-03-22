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
    Tools::log(application, "Handling " + QString(StaticFilesMode == mode ? "static" : "dynamic") + " file: " + path);
    typedef QByteArray *(*GetCacheFunction)(const QString &path);
    typedef bool (*SetCacheFunction)(const QString &path, QByteArray *data);
    if (!Controller::testRequest(application, Controller::GetRequest))
        return;
    if (path.contains("../") || path.contains("/..")) //NOTE: Are you trying to cheat me?
        return Controller::renderNotFound(application);
    GetCacheFunction getCache = (StaticFilesMode == mode) ? &Cache::staticFile : &Cache::dynamicFile;
    SetCacheFunction setCache = (StaticFilesMode == mode) ? &Cache::cacheStaticFile : &Cache::cacheDynamicFile;
    QByteArray *data = getCache(path);
    if (data) {
        application.response().content_type("");
        application.response().out().write(data->data(), data->size());
        return Tools::log(application, "Handled " + QString(StaticFilesMode == mode ? "static" : "dynamic")
                          + " file successfully (cached)");
    }
    QString fn = BDirTools::findResource(Prefix + "/" + path, BDirTools::AllResources);
    if (fn.startsWith(":")) { //NOTE: No need to cache files stored in memory
        bool ok = false;
        QByteArray ba = BDirTools::readFile(fn, -1, &ok);
        if (!ok)
            return Controller::renderNotFound(application);
        application.response().content_type("");
        application.response().out().write(ba.data(), ba.size());
        return Tools::log(application, "Handled " + QString(StaticFilesMode == mode ? "static" : "dynamic")
                          + " file successfully (in-memory)");
    }
    bool ok = false;
    data = new QByteArray(BDirTools::readFile(fn, -1, &ok));
    if (!ok)
        return Controller::renderNotFound(application);
    application.response().content_type("");
    application.response().out().write(data->data(), data->size());
    if (!setCache(path, data))
        delete data;
    Tools::log(application, "Handled " + QString(StaticFilesMode == mode ? "static" : "dynamic")
               + " file successfully");
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
