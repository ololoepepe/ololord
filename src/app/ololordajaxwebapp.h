#ifndef OLOLORDAJAXWEBAPP_H
#define OLOLORDAJAXWEBAPP_H

class AbstractAjaxHandler;

namespace cppcms
{

class service;

}

#include <QList>

#include <cppcms/rpc_json.h>

class OlolordAjaxWebApp : public cppcms::rpc::json_rpc_server
{
private:
    QList<AbstractAjaxHandler *> handlers;
public:
    explicit OlolordAjaxWebApp(cppcms::service &service);
    ~OlolordAjaxWebApp();
};

#endif // OLOLORDAJAXWEBAPP_H
