#include "actionajaxhandler.h"

#include "database.h"
#include "controller/controller.h"
#include "controller/baseboard.h"
#include "tools.h"
#include "translator.h"

#include <BTextTools>

#include <QByteArray>
#include <QDateTime>
#include <QDebug>
#include <QList>
#include <QLocale>
#include <QMap>
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
    o["modificationDateTime"] = post.modificationDateTime;
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
    o["draft"] = post.draft;
    o["rawName"] = post.rawName;
    o["rawSubject"] = post.rawSubject;
    o["text"] = post.text;
    o["rawPostText"] = post.rawPostText;
    o["tripcode"] = post.tripcode;
    cppcms::json::array refs;
    typedef Content::Post::Ref Ref;
    for (std::list<Ref>::const_iterator i = post.referencedBy.begin(); i != post.referencedBy.end(); ++i) {
        cppcms::json::object ref;
        ref["boardName"] = i->boardName;
        ref["postNumber"] = i->postNumber;
        ref["threadNumber"] = i->threadNumber;
        refs.push_back(ref);
    }
    o["referencedBy"] = refs;
    refs.clear();
    for (std::list<Ref>::const_iterator i = post.refersTo.begin(); i != post.refersTo.end(); ++i) {
        cppcms::json::object ref;
        ref["boardName"] = i->boardName;
        ref["postNumber"] = i->postNumber;
        ref["threadNumber"] = i->threadNumber;
        refs.push_back(ref);
    }
    o["refersTo"] = refs;
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
    QString logTarget = sourceBoard + "/" + QString::number(postNumber);
    Tools::log(server, "ajax_ban_user", "begin", logTarget);
    if (!testBan(sourceBoard) || !testBan(board))
        return Tools::log(server, "ajax_ban_user", "fail:ban", logTarget);
    QString reason = Tools::fromStd(params.at("reason").str());
    int level = (int) params.at("level").number();
    QDateTime expires = QDateTime::fromString(Tools::fromStd(params.at("expires").str()), "dd.MM.yyyy:hh");
    QString err;
    if (!Database::banUser(server.request(), sourceBoard, postNumber, board, level, reason, expires, &err)) {
        server.return_error(Tools::toStd(err));
        Tools::log(server, "ajax_ban_user", "fail:" + err, logTarget);
        return;
    }
    server.return_result(true);
    Tools::log(server, "ajax_ban_user", "success", logTarget);
}

void ActionAjaxHandler::deletePost(std::string boardName, long long postNumber, std::string password)
{
    QString bn = Tools::fromStd(boardName);
    quint64 pn = postNumber > 0 ? quint64(postNumber) : 0;
    QString logTarget = bn + "/" + QString::number(pn);
    Tools::log(server, "ajax_delete_post", "begin", logTarget);
    if (!testBan(Tools::fromStd(boardName)))
        return Tools::log(server, "ajax_delete_post", "fail:ban", logTarget);
    QString err;
    if (!Database::deletePost(bn, pn, server.request(), Tools::toHashpass(Tools::fromStd(password)), &err)) {
        server.return_error(Tools::toStd(err));
        Tools::log(server, "ajax_delete_post", "fail:" + err, logTarget);
        return;
    }
    server.return_result(true);
    Tools::log(server, "ajax_delete_post", "success", logTarget);
}

void ActionAjaxHandler::editPost(const cppcms::json::object &params)
{
    QString boardName = Tools::fromStd(params.at("boardName").str());
    long long pn = (long long) params.at("postNumber").number();
    Database::EditPostParameters p(server.request(), boardName, pn > 0 ? quint64(pn) : 0);
    QString logTarget = boardName + "/" + QString::number(p.postNumber);
    Tools::log(server, "ajax_edit_post", "begin", logTarget);
    if (!testBan(boardName))
        return Tools::log(server, "ajax_edit_post", "fail:ban", logTarget);
    p.email = Tools::fromStd(params.at("email").str());
    p.name = Tools::fromStd(params.at("name").str());
    p.raw = params.at("raw").boolean();
    p.subject = Tools::fromStd(params.at("subject").str());
    p.text = Tools::fromStd(params.at("text").str());
    p.password = Tools::toHashpass(Tools::fromStd(params.at("password").str()));
    p.draft = params.at("draft").boolean();
    QString err;
    p.error = &err;
    if (!Database::editPost(p)) {
        server.return_error(Tools::toStd(err));
        Tools::log(server, "ajax_edit_post", "fail:" + err, logTarget);
        return;
    }
    cppcms::json::object refs;
    foreach (const Database::RefKey &key, p.referencedPosts.keys()) {
        std::string k = Tools::toStd(key.boardName) + "/" + Tools::toStd(QString::number(key.postNumber));
        refs[k] = Tools::toStd(QString::number(p.referencedPosts.value(key)));
    }
    server.return_result(refs);
    Tools::log(server, "ajax_edit_post", "success", logTarget);
}

void ActionAjaxHandler::getCaptchaQuota(std::string boardName)
{
    QString bn = Tools::fromStd(boardName);
    QString logTarget = bn;
    Tools::log(server, "ajax_get_captcha_quota", "begin", logTarget);
    if (!testBan(bn, true))
        return Tools::log(server, "ajax_get_captcha_quota", "fail:ban", logTarget);
    AbstractBoard *board = AbstractBoard::board(bn);
    TranslatorStd ts;
    if (!board) {
        std::string err = ts.translate("ActionAjaxHandler", "No such board", "error");
        server.return_error(err);
        Tools::log(server, "ajax_get_captcha_quota", "fail:" + Tools::fromStd(err), logTarget);
        return;
    }
    server.return_result(board->captchaQuota(server.request()));
    Tools::log(server, "ajax_get_captcha_quota", "success", logTarget);
}

void ActionAjaxHandler::getFileExistence(std::string boardName, std::string hash)
{
    QString bn = Tools::fromStd(boardName);
    QString h = Tools::fromStd(hash);
    QString logTarget = bn + "/" + h;
    Tools::log(server, "ajax_get_file_existence", "begin", logTarget);
    if (!testBan(bn, true))
        return Tools::log(server, "ajax_get_file_existence", "fail:ban", logTarget);
    bool ok = false;
    bool exists = Database::fileExists(h, &ok);
    TranslatorStd ts;
    if (!ok) {
        std::string err = ts.translate("ActionAjaxHandler", "Internal database error", "error");
        server.return_error(err);
        Tools::log(server, "ajax_get_file_existence", "fail:" + Tools::fromStd(err), logTarget);
        return;
    }
    server.return_result(exists);
    Tools::log(server, "ajax_get_file_existence", "success", logTarget);
}

void ActionAjaxHandler::getNewPosts(std::string boardName, long long threadNumber, long long lastPostNumber)
{
    QString bn = Tools::fromStd(boardName);
    quint64 tn = threadNumber > 0 ? quint64(threadNumber) : 0;
    quint64 lpn = lastPostNumber > 0 ? quint64(lastPostNumber) : 0;
    QString logTarget = bn + "/" + QString::number(tn) + "/" + QString::number(lpn);
    Tools::log(server, "ajax_get_new_posts", "begin", logTarget);
    if (!testBan(bn, true))
        return Tools::log(server, "ajax_get_new_posts", "fail:ban", logTarget);
    bool ok = false;
    QString err;
    const cppcms::http::request &req = server.request();
    QList<Content::Post> posts = Controller::getNewPosts(req, bn, tn, lpn, &ok, &err);
    if (!ok) {
        server.return_error(Tools::toStd(err));
        Tools::log(server, "ajax_get_new_posts", "fail:" + err, logTarget);
        return;
    }
    cppcms::json::array a;
    foreach (const Content::Post &p, posts)
        a.push_back(toJson(p));
    server.return_result(a);
    Tools::log(server, "ajax_get_new_posts", "success", logTarget);
}

void ActionAjaxHandler::getPost(std::string boardName, long long postNumber)
{
    QString bn = Tools::fromStd(boardName);
    quint64 pn = postNumber > 0 ? quint64(postNumber) : 0;
    QString logTarget = bn + "/" + QString::number(pn);
    Tools::log(server, "ajax_get_post", "begin", logTarget);
    if (!testBan(Tools::fromStd(boardName), true))
        return Tools::log(server, "ajax_get_post", "fail:ban", logTarget);
    bool ok = false;
    QString err;
    const cppcms::http::request &req = server.request();
    Content::Post post = Controller::getPost(req, bn, pn, &ok, &err);
    if (!ok) {
        server.return_error(Tools::toStd(err));
        Tools::log(server, "ajax_get_post", "fail:" + err, logTarget);
        return;
    }
    server.return_result(toJson(post));
    Tools::log(server, "ajax_get_post", "success", logTarget);
}

QList<ActionAjaxHandler::Handler> ActionAjaxHandler::handlers() const
{
    QList<ActionAjaxHandler::Handler> list;
    ActionAjaxHandler *self = const_cast<ActionAjaxHandler *>(this);
    list << Handler("ban_user", cppcms::rpc::json_method(&ActionAjaxHandler::banUser, self), method_role);
    list << Handler("delete_post", cppcms::rpc::json_method(&ActionAjaxHandler::deletePost, self), method_role);
    list << Handler("edit_post", cppcms::rpc::json_method(&ActionAjaxHandler::editPost, self), method_role);
    list << Handler("get_captcha_quota", cppcms::rpc::json_method(&ActionAjaxHandler::getCaptchaQuota, self),
                    method_role);
    list << Handler("get_file_existence", cppcms::rpc::json_method(&ActionAjaxHandler::getFileExistence, self),
                    method_role);
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
    QString bn = Tools::fromStd(boardName);
    quint64 pn = postNumber > 0 ? quint64(postNumber) : 0;
    QString logTarget = bn + "/" + QString::number(pn) + "/" + QString(fixed ? "true" : "false");
    Tools::log(server, "ajax_set_thread_fixed", "begin", logTarget);
    if (!testBan(Tools::fromStd(boardName)))
        return Tools::log(server, "ajax_set_thread_fixed", "fail:ban", logTarget);
    QString err;
    if (!Database::setThreadFixed(bn, pn, fixed, server.request(), &err)) {
        server.return_error(Tools::toStd(err));
        Tools::log(server, "ajax_set_thread_fixed", "fail:" + err, logTarget);
        return;
    }
    server.return_result(true);
    Tools::log(server, "ajax_set_thread_fixed", "success", logTarget);
}

void ActionAjaxHandler::setThreadOpened(std::string boardName, long long postNumber, bool opened)
{
    QString bn = Tools::fromStd(boardName);
    quint64 pn = postNumber > 0 ? quint64(postNumber) : 0;
    QString logTarget = bn + "/" + QString::number(pn) + "/" + QString(opened ? "true" : "false");
    Tools::log(server, "ajax_set_thread_opened", "begin", logTarget);
    if (!testBan(Tools::fromStd(boardName)))
        return Tools::log(server, "ajax_set_thread_opened", "fail:ban", logTarget);
    QString err;
    if (!Database::setThreadOpened(bn, pn, opened, server.request(), &err)) {
        server.return_error(Tools::toStd(err));
        Tools::log(server, "ajax_set_thread_opened", "fail:" + err, logTarget);
        return;
    }
    server.return_result(true);
    Tools::log(server, "ajax_set_thread_opened", "success", logTarget);
}
