#ifndef ACTIONAJAXHANDLER_H
#define ACTIONAJAXHANDLER_H

namespace cppcms
{

namespace rpc
{

class json_rpc_server;

}

}

#include "abstractajaxhandler.h"

#include <QList>

#include <string>

class OLOLORD_EXPORT ActionAjaxHandler : public AbstractAjaxHandler
{
public:
    explicit ActionAjaxHandler(cppcms::rpc::json_rpc_server &srv);
public:
    void deletePost(std::string boardName, long long postNumber, std::string pwd);
    QList<Handler> handlers() const;
};

#endif // ACTIONAJAXHANDLER_H
