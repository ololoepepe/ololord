#ifndef ABSTRACTAJAXHANDLER_H
#define ABSTRACTAJAXHANDLER_H

#include "../global.h"

#include <QList>
#include <QString>

#include <cppcms/rpc_json.h>

#include <string>

class OLOLORD_EXPORT AbstractAjaxHandler
{
public:
    typedef cppcms::rpc::json_rpc_server::method_type method_type;
    typedef cppcms::rpc::json_rpc_server::role_type role_type;
public:
    static const role_type any_role;
    static const role_type method_role;
    static const role_type notification_role;
public:
    struct OLOLORD_EXPORT Handler
    {
        method_type method;
        QString name;
        role_type role;
    public:
        explicit Handler(const QString &nm, const method_type &m, role_type r = any_role)
        {
            name = nm;
            method = m;
            role = r;
        }
    };
protected:
    cppcms::rpc::json_rpc_server &server;
public:
    explicit AbstractAjaxHandler(cppcms::rpc::json_rpc_server &srv);
    virtual ~AbstractAjaxHandler();
public:
    virtual QList<Handler> handlers() const = 0;
protected:
    bool testBan(const QString &boardName);
};

#endif // ABSTRACTAJAXHANDLER_H
