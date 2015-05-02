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
    typedef Tools::StaticFile *(*GetCacheFunction)(const QString &path);
    typedef bool (*SetCacheFunction)(const QString &path, Tools::StaticFile *file);
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
    Tools::StaticFile *file = getCache(path);
    if (file) {
        write(file->data, file->mimeType);
        Tools::log(application, logAction, "success:cache", logTarget);
        return;
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
        write(ba, Tools::toStd(Tools::mimeType(ba)));
        Tools::log(application, logAction, "success:in_memory", logTarget);
        return;
    }
    bool ok = false;
    file = new Tools::StaticFile;
    file->data = BDirTools::readFile(fn, -1, &ok);
    file->mimeType = Tools::toStd(Tools::mimeType(file->data));
    if (!ok) {
        Controller::renderNotFound(application);
        Tools::log(application, logAction, "fail:not_found", logTarget);
        return;
    }
    write(file->data, file->mimeType);
    if (!setCache(path, file))
        delete file;
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

void StaticFilesRoute::write(const QByteArray &data, const std::string &contentType)
{
    application.response().content_length(data.size());
    if ("text/plain" == contentType)
        application.response().content_type("");
    else
        application.response().content_type(contentType);
    application.response().out().write(data.data(), data.size());
}
