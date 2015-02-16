#include "abstractajaxhandler.h"

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
