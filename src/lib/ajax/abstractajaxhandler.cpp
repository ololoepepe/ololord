#include "abstractajaxhandler.h"

#include "database.h"
#include "tools.h"
#include "translator.h"

#include <cppcms/rpc_json.h>

const AbstractAjaxHandler::role_type AbstractAjaxHandler::any_role = cppcms::rpc::json_rpc_server::any_role;
const AbstractAjaxHandler::role_type AbstractAjaxHandler::method_role = cppcms::rpc::json_rpc_server::method_role;
const AbstractAjaxHandler::role_type AbstractAjaxHandler::notification_role =
        cppcms::rpc::json_rpc_server::notification_role;

AbstractAjaxHandler::AbstractAjaxHandler(cppcms::rpc::json_rpc_server &srv) :
    server(srv)
{
    //
}

AbstractAjaxHandler::~AbstractAjaxHandler()
{
    //
}

bool AbstractAjaxHandler::testBan(const QString &boardName)
{
    TranslatorStd ts(server.request());
    QString ip = Tools::userIp(server.request());
    bool ok = false;
    QString err;
    Database::BanInfo inf = Database::userBanInfo(ip, boardName, &ok, &err, ts.locale());
    if (!ok) {
        server.return_error(Tools::toStd(err));
        return false;
    }
    if (inf.level >= 10) {
        server.return_error(ts.translate("AbstractAjaxHandler", "You are banned", "error"));
        return false;
    }
    return true;
}
