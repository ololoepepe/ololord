#ifndef AJAXHANDLERFACTORYPLUGININTERFACE_H
#define AJAXHANDLERFACTORYPLUGININTERFACE_H

namespace cppcms
{

namespace rpc
{

class json_rpc_server;

}

}

#include "ajax/abstractajaxhandler.h"

#include <QList>
#include <QtPlugin>

/*============================================================================
================================ AjaxHandlerFactoryPluginInterface ===========
============================================================================*/

class OLOLORD_EXPORT AjaxHandlerFactoryPluginInterface
{
public:
    virtual ~AjaxHandlerFactoryPluginInterface() {}
public:
    virtual QList<AbstractAjaxHandler *> createAjaxHandlers(cppcms::rpc::json_rpc_server &srv) = 0;
};

Q_DECLARE_INTERFACE(AjaxHandlerFactoryPluginInterface, "ololord.AjaxHandlerFactoryPluginInterface")

#endif // AJAXHANDLERFACTORYPLUGININTERFACE_H
