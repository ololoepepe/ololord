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

#include <cppcms/json.h>

#include <map>
#include <string>

class OLOLORD_EXPORT ActionAjaxHandler : public AbstractAjaxHandler
{
public:
    explicit ActionAjaxHandler(cppcms::rpc::json_rpc_server &srv);
public:
    void banUser(const cppcms::json::object &params);
    void deletePost(std::string boardName, long long postNumber, std::string password);
    void editPost(std::string boardName, long long postNumber, std::string text);
    void getNewPosts(std::string boardName, long long threadNumber, long long lastPostNumber);
    void getPost(std::string boardName, long long postNumber);
    QList<Handler> handlers() const;
    void setThreadFixed(std::string boardName, long long postNumber, bool fixed);
    void setThreadOpened(std::string boardName, long long postNumber, bool opened);
};

#endif // ACTIONAJAXHANDLER_H
