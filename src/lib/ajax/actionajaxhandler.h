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
    void deletePost(std::string boardName, long long postNumber, std::string password);
    QList<Handler> handlers() const;
    void setThreadFixed(std::string boardName, long long postNumber, bool fixed);
    void setThreadOpened(std::string boardName, long long postNumber, bool opened);
};

#endif // ACTIONAJAXHANDLER_H
