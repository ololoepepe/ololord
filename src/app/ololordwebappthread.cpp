#include "ololordwebappthread.h"

#include "ololordwebapp.h"

#include <QDebug>
#include <QThread>

#include <cppcms/applications_pool.h>
#include <cppcms/json.h>
#include <cppcms/service.h>

#include <exception>

OlolordWebAppThread::OlolordWebAppThread(const cppcms::json::value &conf, QObject *parent) :
    QThread(parent), Conf(conf)
{
    //
}

void OlolordWebAppThread::run()
{
    forever {
        try {
            cppcms::service service(Conf);
            service.applications_pool().mount(cppcms::applications_factory<OlolordWebApp>());
            service.run();
        } catch(std::exception const &e) {
            qDebug() << e.what();
        }
    }
}
