#include "ololordajaxwebapp.h"

#include <ajax/AbstractAjaxHandler>
#include <ajax/ActionAjaxHandler>
#include <plugin/AjaxHandlerFactoryPluginInterface>
#include <Tools>

#include <BCoreApplication>
#include <BPluginInterface>
#include <BPluginWrapper>

#include <QDebug>
#include <QList>
#include <QString>

#include <cppcms/rpc_json.h>
#include <cppcms/service.h>

#include <string>

OlolordAjaxWebApp::OlolordAjaxWebApp(cppcms::service &service) :
    cppcms::rpc::json_rpc_server(service)
{
    AbstractAjaxHandler *ah = new ActionAjaxHandler(*this);
    foreach (const AbstractAjaxHandler::Handler &h, ah->handlers())
        bind(Tools::toStd(h.name), h.method, h.role);
    handlers << ah;
    foreach (BPluginWrapper *pw, BCoreApplication::pluginWrappers("ajax-handler-factory")) {
        AjaxHandlerFactoryPluginInterface *i = qobject_cast<AjaxHandlerFactoryPluginInterface *>(pw->instance());
        if (!i)
            continue;
        foreach (AbstractAjaxHandler *r, i->createAjaxHandlers(*this)) {
            if (!r)
                continue;
            bool installed = false;
            foreach (const AbstractAjaxHandler::Handler &h, r->handlers()) {
                if (h.name.isEmpty())
                    continue;
                bind(Tools::toStd(h.name), h.method, h.role);
                installed = true;
            }
            if (!installed)
                delete r;
            else
                handlers << r;
        }
    }
}

OlolordAjaxWebApp::~OlolordAjaxWebApp()
{
    foreach (AbstractAjaxHandler *h, handlers)
        delete h;
    handlers.clear();
}
