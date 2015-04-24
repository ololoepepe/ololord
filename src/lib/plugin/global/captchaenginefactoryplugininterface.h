#ifndef CAPTCHAENGINEFACTORYPLUGININTERFACE_H
#define CAPTCHAENGINEFACTORYPLUGININTERFACE_H

#include "route/abstractroute.h"

#include <QList>
#include <QtPlugin>

/*============================================================================
================================ CaptchaEngineFactoryPluginInterface =========
============================================================================*/

class OLOLORD_EXPORT CaptchaEngineFactoryPluginInterface
{
public:
    virtual ~CaptchaEngineFactoryPluginInterface() {}
public:
    virtual QList<AbstractCaptchaEngine *> createCaptchaEngines() = 0;
};

Q_DECLARE_INTERFACE(CaptchaEngineFactoryPluginInterface, "ololord.CaptchaEngineFactoryPluginInterface")

#endif // CAPTCHAENGINEFACTORYPLUGININTERFACE_H
