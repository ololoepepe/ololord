#ifndef OLOLORDWEBAPP_H
#define OLOLORDWEBAPP_H

class AbstractRoute;

namespace cppcms
{

class service;

}

#include <QList>

#include <cppcms/application.h>

#include <string>

class OlolordWebApp : public cppcms::application
{
private:
    QList<AbstractRoute *> routes;
public:
    explicit OlolordWebApp(cppcms::service &service);
    ~OlolordWebApp();
public:
    void main(std::string url);
};

#endif // OLOLORDWEBAPP_H
