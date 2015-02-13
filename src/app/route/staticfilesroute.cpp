#include "staticfilesroute.h"

#include <cache.h>
#include <controller/controller.h>
#include <tools.h>

#include <BDirTools>
#include <BeQt>

#include <QByteArray>
#include <QDebug>
#include <QString>

#include <cppcms/application.h>
#include <cppcms/http_request.h>
#include <cppcms/http_response.h>

StaticFilesRoute::StaticFilesRoute(const QString &prefix) :
    Prefix(prefix)
{
    //
}

void StaticFilesRoute::handle(cppcms::application &app, const QString &path)
{
    Tools::log(app, "Handling static file: " + path);
    typedef QByteArray *(*GetCacheFunction)(const QString &path);
    typedef bool (*SetCacheFunction)(const QString &path, QByteArray *data);
    if (!Controller::testRequest(app, Controller::GetRequest))
        return;
    if (path.contains("../") || path.contains("/..")) //NOTE: Are you trying to cheat me?
        return Controller::renderNotFound(app);
    GetCacheFunction getCache = Prefix.startsWith("static") ? &Cache::staticFile : &Cache::dynamicFile;
    SetCacheFunction setCache = Prefix.startsWith("static") ? &Cache::cacheStaticFile : &Cache::cacheDynamicFile;
    QByteArray *data = getCache(path);
    if (data) {
        app.response().content_type("");
        app.response().out().write(data->data(), data->size());
        return Tools::log(app, "Handled static file successfully (cached)");
    }
    QString fn = BDirTools::findResource(Prefix + "/" + path, BDirTools::AllResources);
    if (fn.startsWith(":")) { //NOTE: No need to cache files stored in memory
        bool ok = false;
        QByteArray ba = BDirTools::readFile(fn, -1, &ok);
        if (!ok)
            return Controller::renderNotFound(app);
        app.response().content_type("");
        app.response().out().write(ba.data(), ba.size());
        return Tools::log(app, "Handled static file successfully (in-memory)");
    }
    bool ok = false;
    data = new QByteArray(BDirTools::readFile(fn, -1, &ok));
    if (!ok)
        return Controller::renderNotFound(app);
    app.response().content_type("");
    app.response().out().write(data->data(), data->size());
    if (!setCache(path, data))
        delete data;
    Tools::log(app, "Handled static file successfully");
}
