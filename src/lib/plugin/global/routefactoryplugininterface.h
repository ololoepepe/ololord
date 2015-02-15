#ifndef ROUTEFACTORYPLUGININTERFACE_H
#define ROUTEFACTORYPLUGININTERFACE_H

namespace cppcms
{

class application;

}

#include "route/abstractroute.h"

#include <QList>
#include <QtPlugin>

/*============================================================================
================================ RouteFactoryPluginInterface =================
============================================================================*/

class OLOLORD_EXPORT RouteFactoryPluginInterface
{
public:
    virtual ~RouteFactoryPluginInterface() {}
public:
    virtual QList<AbstractRoute *> createRoutes(cppcms::application &app) = 0;
};

Q_DECLARE_INTERFACE(RouteFactoryPluginInterface, "ololord.RouteFactoryPluginInterface")

#endif // ROUTEFACTORYPLUGININTERFACE_H
