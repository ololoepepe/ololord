#include "ololordwebappthread.h"

#include "ololordwebapp.h"

#include <tools.h>

#include <QDebug>
#include <QThread>

#include <cppcms/applications_pool.h>
#include <cppcms/json.h>
#include <cppcms/service.h>

#include <exception>

OlolordWebAppThread::OlolordWebAppThread(const cppcms::json::value &conf, QObject *parent) :
    QThread(parent), Conf(conf)
{
    mservice = 0;
    mshutdown = false;
}

void OlolordWebAppThread::run()
{
    while (!mshutdown) {
        try {
            cppcms::service service(Conf);
            mservice = &service;
            service.applications_pool().mount(cppcms::applications_factory<OlolordWebApp>());
            service.run();
        } catch(std::exception const &e) {
            Tools::log("OlolordWebAppThread::run", e);
        }
        mservice = 0;
    }
}

void OlolordWebAppThread::shutdown()
{
    if (!mservice)
        return;
    mshutdown = true;
    mservice->shutdown();
}
