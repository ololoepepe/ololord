#ifndef OLOLORDWEBAPPTHREAD_H
#define OLOLORDWEBAPPTHREAD_H

namespace cppcms
{

class service;

}

#include <QThread>

#include <cppcms/json.h>

class OlolordWebAppThread : public QThread
{
private:
    const cppcms::json::value Conf;
private:
    cppcms::service *mservice;
    bool mshutdown;
public:
    explicit OlolordWebAppThread(const cppcms::json::value &conf, QObject *parent = 0);
public:
    void shutdown();
protected:
    void run();
};

#endif // OLOLORDWEBAPPTHREAD_H
