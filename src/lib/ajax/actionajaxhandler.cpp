#include "actionajaxhandler.h"

#include "database.h"
#include "tools.h"

#include <QByteArray>
#include <QDebug>
#include <QList>
#include <QLocale>
#include <QString>

#include <cppcms/json.h>
#include <cppcms/rpc_json.h>

#include <string>

ActionAjaxHandler::ActionAjaxHandler(cppcms::rpc::json_rpc_server &srv) :
    AbstractAjaxHandler(srv)
{
    //
}

void ActionAjaxHandler::deletePost(std::string boardName, long long postNumber, std::string password)
{
    QString err;
    if (!Database::deletePost(Tools::fromStd(boardName), postNumber > 0 ? quint64(postNumber) : 0, server.request(),
                              Tools::toHashpass(Tools::fromStd(password)), &err)) {
        return server.return_error(Tools::toStd(err));
    }
    server.return_result(true);
}

QList<ActionAjaxHandler::Handler> ActionAjaxHandler::handlers() const
{
    QList<ActionAjaxHandler::Handler> list;
    ActionAjaxHandler *self = const_cast<ActionAjaxHandler *>(this);
    list << Handler("delete_post", cppcms::rpc::json_method(&ActionAjaxHandler::deletePost, self), method_role);
    return list;
}
