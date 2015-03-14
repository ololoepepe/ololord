#include "actionajaxhandler.h"

#include "database.h"
#include "controller/controller.h"
#include "controller/baseboard.h"
#include "tools.h"

#include <BTextTools>

#include <QByteArray>
#include <QDateTime>
#include <QDebug>
#include <QList>
#include <QLocale>
#include <QString>
#include <QStringList>

#include <cppcms/json.h>
#include <cppcms/rpc_json.h>

#include <string>

static cppcms::json::object toJson(const Content::Post &post)
{
    cppcms::json::object o;
    o["bannedFor"] = post.bannedFor;
    o["cityName"] = post.cityName;
    o["closed"] = post.closed;
    o["countryName"] = post.countryName;
    o["dateTime"] = post.dateTime;
    o["email"] = post.email;
    cppcms::json::array files;
    for (std::list<Content::File>::const_iterator i = post.files.begin(); i != post.files.end(); ++i) {
        const Content::File &file = *i;
        cppcms::json::object f;
        f["type"] = file.type;
        f["size"] = file.size;
        f["thumbSizeX"] = file.thumbSizeX;
        f["thumbSizeY"] = file.thumbSizeY;
        f["sizeX"] = file.sizeX;
        f["sizeY"] = file.sizeY;
        f["sourceName"] = file.sourceName;
        f["thumbName"] = file.thumbName;
        files.push_back(f);
    }
    o["files"] = files;
    o["fixed"] = post.fixed;
    o["flagName"] = post.flagName;
    o["hidden"] = post.hidden;
    o["ip"] = post.ip;
    o["name"] = post.name;
    o["nameRaw"] = post.nameRaw;
    o["number"] = post.number;
    o["showRegistered"] = post.showRegistered;
    o["showTripcode"] = post.showTripcode;
    o["threadNumber"] = post.threadNumber;
    o["subject"] = post.subject;
    o["subjectIsRaw"] = post.subjectIsRaw;
    o["text"] = post.text;
    o["rawPostText"] = post.rawPostText;
    o["tripcode"] = post.tripcode;
    return o;
}

ActionAjaxHandler::ActionAjaxHandler(cppcms::rpc::json_rpc_server &srv) :
    AbstractAjaxHandler(srv)
{
    //
}

void ActionAjaxHandler::banUser(const cppcms::json::object &params)
{
    QString sourceBoard = Tools::fromStd(params.at("boardName").str());
    long long pn = (long long) params.at("postNumber").number();
    quint64 postNumber = pn > 0 ? quint64(pn) : 0;
    QString board = Tools::fromStd(params.at("board").str());
    if (!testBan(sourceBoard) || !testBan(board))
        return;
    QString reason = Tools::fromStd(params.at("reason").str());
    int level = (int) params.at("level").number();
    QDateTime expires = QDateTime::fromString(Tools::fromStd(params.at("expires").str()), "dd.MM.yyyy:hh");
    QString err;
    if (!Database::banUser(server.request(), sourceBoard, postNumber, board, level, reason, expires, &err))
        return server.return_error(Tools::toStd(err));
    server.return_result(true);
}

void ActionAjaxHandler::deletePost(std::string boardName, long long postNumber, std::string password)
{
    if (!testBan(Tools::fromStd(boardName)))
        return;
    QString err;
    if (!Database::deletePost(Tools::fromStd(boardName), postNumber > 0 ? quint64(postNumber) : 0, server.request(),
                              Tools::toHashpass(Tools::fromStd(password)), &err)) {
        return server.return_error(Tools::toStd(err));
    }
    server.return_result(true);
}

void ActionAjaxHandler::editPost(std::string boardName, long long postNumber, std::string text)
{
    if (!testBan(Tools::fromStd(boardName)))
        return;
    QString err;
    if (!Database::editPost(server.request(), Tools::fromStd(boardName), postNumber > 0 ? quint64(postNumber) : 0,
                            Tools::fromStd(text), &err)) {
        return server.return_error(Tools::toStd(err));
    }
    server.return_result(true);
}

void ActionAjaxHandler::getNewPosts(std::string boardName, long long threadNumber, long long lastPostNumber)
{
    if (!testBan(Tools::fromStd(boardName), true))
        return;
    bool ok = false;
    QString err;
    const cppcms::http::request &req = server.request();
    QList<Content::Post> posts = Controller::getNewPosts(req, Tools::fromStd(boardName),
        threadNumber > 0 ? quint64(threadNumber) : 0, lastPostNumber > 0 ? quint64(lastPostNumber) : 0, &ok, &err);
    if (!ok)
        return server.return_error(Tools::toStd(err));
    cppcms::json::array a;
    foreach (const Content::Post &p, posts)
        a.push_back(toJson(p));
    server.return_result(a);
}

void ActionAjaxHandler::getPost(std::string boardName, long long postNumber)
{
    if (!testBan(Tools::fromStd(boardName), true))
        return;
    bool ok = false;
    QString err;
    const cppcms::http::request &req = server.request();
    Content::Post post = Controller::getPost(req, Tools::fromStd(boardName), postNumber > 0 ? quint64(postNumber) : 0,
                                             &ok, &err);
    if (!ok)
        return server.return_error(Tools::toStd(err));
    server.return_result(toJson(post));
}

QList<ActionAjaxHandler::Handler> ActionAjaxHandler::handlers() const
{
    QList<ActionAjaxHandler::Handler> list;
    ActionAjaxHandler *self = const_cast<ActionAjaxHandler *>(this);
    list << Handler("ban_user", cppcms::rpc::json_method(&ActionAjaxHandler::banUser, self), method_role);
    list << Handler("delete_post", cppcms::rpc::json_method(&ActionAjaxHandler::deletePost, self), method_role);
    list << Handler("edit_post", cppcms::rpc::json_method(&ActionAjaxHandler::editPost, self), method_role);
    list << Handler("get_new_posts", cppcms::rpc::json_method(&ActionAjaxHandler::getNewPosts, self), method_role);
    list << Handler("get_post", cppcms::rpc::json_method(&ActionAjaxHandler::getPost, self), method_role);
    list << Handler("set_thread_fixed", cppcms::rpc::json_method(&ActionAjaxHandler::setThreadFixed, self),
                    method_role);
    list << Handler("set_thread_opened", cppcms::rpc::json_method(&ActionAjaxHandler::setThreadOpened, self),
                    method_role);
    return list;
}

void ActionAjaxHandler::setThreadFixed(std::string boardName, long long postNumber, bool fixed)
{
    if (!testBan(Tools::fromStd(boardName)))
        return;
    QString err;
    if (!Database::setThreadFixed(Tools::fromStd(boardName), postNumber > 0 ? quint64(postNumber) : 0, fixed,
                                  server.request(), &err)) {
        return server.return_error(Tools::toStd(err));
    }
    server.return_result(true);
}

void ActionAjaxHandler::setThreadOpened(std::string boardName, long long postNumber, bool opened)
{
    if (!testBan(Tools::fromStd(boardName)))
        return;
    QString err;
    if (!Database::setThreadOpened(Tools::fromStd(boardName), postNumber > 0 ? quint64(postNumber) : 0, opened,
                                   server.request(), &err)) {
        return server.return_error(Tools::toStd(err));
    }
    server.return_result(true);
}
