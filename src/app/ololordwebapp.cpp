#include "ololordwebapp.h"

#include "ololordajaxwebapp.h"

#include <Controller>
#include <plugin/RouteFactoryPluginInterface>
#include <route/AbstractRoute>
#include <route/ActionRoute>
#include <route/AddFileRoute>
#include <route/BanUserRoute>
#include <route/BoardRoute>
#include <route/CatalogRoute>
#include <route/EditAudioTagsRoute>
#include <route/EditPostRoute>
#include <route/FaqRoute>
#include <route/FrameListRoute>
#include <route/FrameRoute>
#include <route/HomeRoute>
#include <route/MarkupRoute>
#include <route/MoveThreadRoute>
#include <route/PlaylistRoute>
#include <route/RssRoute>
#include <route/SearchRoute>
#include <route/SettingsRoute>
#include <route/StaticFilesRoute>
#include <route/ThreadRoute>
#include <SettingsLocker>
#include <tools.h>

#include <BCoreApplication>
#include <BDirTools>
#include <BPluginWrapper>

#include <QDebug>
#include <QList>
#include <QMap>
#include <QSettings>
#include <QString>
#include <QStringList>
#include <QtAlgorithms>
#include <QVariant>

#include <cppcms/application.h>
#include <cppcms/service.h>
#include <cppcms/url_dispatcher.h>
#include <cppcms/url_mapper.h>

static bool routeLessThan(AbstractRoute *r1, AbstractRoute *r2)
{
    if (!r1 || !r2)
        return false;
    return r1->priority() < r2->priority();
}

OlolordWebApp::OlolordWebApp(cppcms::service &service) :
    cppcms::application(service)
{
    attach(new OlolordAjaxWebApp(service), "/api", 0);
    QMap<std::string, AbstractRoute *> routesMap;
    AbstractRoute *r = new ActionRoute(*this);
    routesMap.insert(r->regex(), r);
    r = new AddFileRoute(*this);
    routesMap.insert(r->regex(), r);
    r = new BoardRoute(*this, BoardRoute::BoardMode);
    routesMap.insert(r->regex(), r);
    r = new BoardRoute(*this, BoardRoute::BoardPageMode);
    routesMap.insert(r->regex(), r);
    r = new BoardRoute(*this, BoardRoute::BoardRulesRoute);
    routesMap.insert(r->regex(), r);
    r = new BanUserRoute(*this);
    routesMap.insert(r->regex(), r);
    r = new CatalogRoute(*this);
    routesMap.insert(r->regex(), r);
    r = new EditAudioTagsRoute(*this);
    routesMap.insert(r->regex(), r);
    r = new EditPostRoute(*this);
    routesMap.insert(r->regex(), r);
    r = new FaqRoute(*this);
    routesMap.insert(r->regex(), r);
    r = new FrameListRoute(*this);
    routesMap.insert(r->regex(), r);
    r = new FrameRoute(*this);
    routesMap.insert(r->regex(), r);
    r = new HomeRoute(*this);
    routesMap.insert(r->regex(), r);
    r = new MarkupRoute(*this);
    routesMap.insert(r->regex(), r);
    r = new MoveThreadRoute(*this);
    routesMap.insert(r->regex(), r);
    r = new PlaylistRoute(*this);
    routesMap.insert(r->regex(), r);
    r = new RssRoute(*this);
    routesMap.insert(r->regex(), r);
    r = new SearchRoute(*this);
    routesMap.insert(r->regex(), r);
    r = new SettingsRoute(*this);
    routesMap.insert(r->regex(), r);
    r = new StaticFilesRoute(*this, StaticFilesRoute::StaticFilesMode);
    routesMap.insert(r->regex(), r);
    r = new StaticFilesRoute(*this, StaticFilesRoute::DynamicFilesMode);
    routesMap.insert(r->regex(), r);
    r = new ThreadRoute(*this);
    routesMap.insert(r->regex(), r);
    foreach (BPluginWrapper *pw, BCoreApplication::pluginWrappers("route-factory")) {
        RouteFactoryPluginInterface *i = qobject_cast<RouteFactoryPluginInterface *>(pw->instance());
        if (!i)
            continue;
        foreach (AbstractRoute *r, i->createRoutes(*this)) {
            if (!r)
                continue;
            std::string rx = r->regex();
            if (rx.empty() || r->handlerArgumentCount() > 4) {
                delete r;
                continue;
            }
            if (routesMap.contains(rx))
                delete routesMap.take(rx);
            routesMap.insert(rx, r);
        }
    }
    routes = routesMap.values();
    qSort(routes.begin(), routes.end(), &routeLessThan);
    foreach (AbstractRoute *r, routes) {
        switch (r->handlerArgumentCount()) {
        case 1:
            dispatcher().assign(r->regex(), &AbstractRoute::handle, r, 1);
            if (r->duplicateWithSlashAppended())
                dispatcher().assign(r->regex() + "/", &AbstractRoute::handle, r, 1);
            break;
        case 2:
            dispatcher().assign(r->regex(), &AbstractRoute::handle, r, 1, 2);
            if (r->duplicateWithSlashAppended())
                dispatcher().assign(r->regex() + "/", &AbstractRoute::handle, r, 1, 2);
            break;
        case 3:
            dispatcher().assign(r->regex(), &AbstractRoute::handle, r, 1, 2, 3);
            if (r->duplicateWithSlashAppended())
                dispatcher().assign(r->regex() + "/", &AbstractRoute::handle, r, 1, 2, 3);
            break;
        case 4:
            dispatcher().assign(r->regex(), &AbstractRoute::handle, r, 1, 2, 3, 4);
            if (r->duplicateWithSlashAppended())
                dispatcher().assign(r->regex() + "/", &AbstractRoute::handle, r, 1, 2, 3, 4);
            break;
        case 0:
        default:
            dispatcher().assign(r->regex(), &AbstractRoute::handle, r);
            if (r->duplicateWithSlashAppended())
                dispatcher().assign(r->regex() + "/", &AbstractRoute::handle, r);
            break;
        }
        if (!r->key().empty()) {
            mapper().assign(r->key(), r->url());
            if (r->duplicateWithSlashAppended())
                mapper().assign(r->key(), r->url() + "/");
        } else {
            mapper().assign("");
        }
    }
    QString path = SettingsLocker()->value("Site/path_prefix").toString();
    if (path.endsWith("/"))
        path.remove(path.length() - 1, 1);
    if (!path.isEmpty())
        mapper().root(Tools::toStd("/" + path));
}

OlolordWebApp::~OlolordWebApp()
{
    foreach (AbstractRoute *r, routes)
        delete r;
    routes.clear();
}

void OlolordWebApp::main(std::string url)
{
    try {
        if (dispatcher().dispatch(url))
            return;
        Controller::renderNotFoundNonAjax(*this);
        Tools::log(*this, Tools::fromStd(url), "fail:not_found:handled_by_main");
    } catch (const std::exception &e) {
        Tools::log("OlolordWebApp::main", e);
    }
}
