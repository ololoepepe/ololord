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

void ActionAjaxHandler::deletePost(std::string boardName, long long postNumber, std::string pwd)
{
    QString board = Tools::fromStd(boardName);
    quint64 post = postNumber > 0 ? quint64(postNumber) : 0;
    QByteArray password = Tools::toHashpass(Tools::fromStd(pwd));
    const cppcms::http::request &req = server.request();
    QLocale l = Tools::locale(req);
    QString err;
    if (!Database::mayDeletePost(board, post, req, password, &err, l))
        return server.return_result(Tools::toStd(err));
    if (!Database::deletePost(board, post, &err, l))
        return server.return_result(Tools::toStd(err));
    server.return_result("");
}

QList<ActionAjaxHandler::Handler> ActionAjaxHandler::handlers() const
{
    QList<ActionAjaxHandler::Handler> list;
    ActionAjaxHandler *self = const_cast<ActionAjaxHandler *>(this);
    list << Handler("delete_post", cppcms::rpc::json_method(&ActionAjaxHandler::deletePost, self), method_role);
    return list;
}
