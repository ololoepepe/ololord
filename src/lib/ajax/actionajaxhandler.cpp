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
    list << Handler("set_thread_fixed", cppcms::rpc::json_method(&ActionAjaxHandler::setThreadFixed, self),
                    method_role);
    list << Handler("set_thread_opened", cppcms::rpc::json_method(&ActionAjaxHandler::setThreadOpened, self),
                    method_role);
    return list;
}

void ActionAjaxHandler::setThreadFixed(std::string boardName, long long postNumber, bool fixed)
{
    QString err;
    if (!Database::setThreadFixed(Tools::fromStd(boardName), postNumber > 0 ? quint64(postNumber) : 0, fixed,
                                  server.request(), &err)) {
        return server.return_error(Tools::toStd(err));
    }
    server.return_result(true);
}

void ActionAjaxHandler::setThreadOpened(std::string boardName, long long postNumber, bool opened)
{
    QString err;
    if (!Database::setThreadOpened(Tools::fromStd(boardName), postNumber > 0 ? quint64(postNumber) : 0, opened,
                                   server.request(), &err)) {
        return server.return_error(Tools::toStd(err));
    }
    server.return_result(true);
}
