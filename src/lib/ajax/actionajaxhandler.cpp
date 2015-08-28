#include "actionajaxhandler.h"

#include "database.h"
#include "captcha/abstractcaptchaengine.h"
#include "captcha/abstractyandexcaptchaengine.h"
#include "controller.h"
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
#include <QRegExp>
#include <QString>
#include <QStringList>
#include <QUrl>

#include <cppcms/http_cookie.h>
#include <cppcms/json.h>
#include <cppcms/rpc_json.h>

#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>

#include <climits>
#include <sstream>
#include <string>

ActionAjaxHandler::ActionAjaxHandler(cppcms::rpc::json_rpc_server &srv) :
    AbstractAjaxHandler(srv)
{
    //
}

void ActionAjaxHandler::banPoster(const cppcms::json::object &params)
{
    try {
        QString sourceBoard = Tools::fromStd(params.at("boardName").str());
        long long pn = (long long) params.at("postNumber").number();
        quint64 postNumber = pn > 0 ? quint64(pn) : 0;
        QString logTarget = sourceBoard + "/" + QString::number(postNumber);
        Tools::log(server, "ajax_ban_poster", "begin", logTarget);
        if (!testBan(sourceBoard))
            return Tools::log(server, "ajax_ban_poster", "fail:ban", logTarget);
        QList<Database::BanInfo> bans;
        foreach (const cppcms::json::value &v, params.at("bans").array()) {
            cppcms::json::object o = v.object();
            Database::BanInfo inf;
            inf.boardName = Tools::fromStd(o.at("boardName").str());
            inf.expires = QDateTime::fromString(Tools::fromStd(o.at("expires").str()), "dd.MM.yyyy:hh");
            inf.level = int(o.at("level").number());
            inf.reason = Tools::fromStd(o.at("reason").str());
            bans << inf;
        }
        QString err;
        if (!Database::banPoster(server.request(), sourceBoard, postNumber, bans, &err)) {
            server.return_error(Tools::toStd(err));
            Tools::log(server, "ajax_ban_poster", "fail:" + err, logTarget);
            return;
        }
        server.return_result(true);
        Tools::log(server, "ajax_ban_poster", "success", logTarget);
    } catch (const cppcms::json::bad_value_cast &e) {
        QString err = Tools::fromStd(e.what());
        server.return_error(Tools::toStd(err));
        Tools::log(server, "ajax_ban_poster", "fail:" + err);
    } catch (const std::out_of_range &e) {
        QString err = Tools::fromStd(e.what());
        server.return_error(Tools::toStd(err));
        Tools::log(server, "ajax_ban_poster", "fail:" + err);
    } catch (const std::exception &e) {
        QString err = Tools::fromStd(e.what());
        server.return_error(Tools::toStd(err));
        Tools::log(server, "ajax_ban_poster", "fail:" + err);
    }
}

void ActionAjaxHandler::banUser(const cppcms::json::object &params)
{
    try {
        QString ip = Tools::fromStd(params.at("ip").str());
        QString logTarget = ip;
        Tools::log(server, "ajax_ban_user", "begin", logTarget);
        QList<Database::BanInfo> bans;
        foreach (const cppcms::json::value &v, params.at("bans").array()) {
            cppcms::json::object o = v.object();
            Database::BanInfo inf;
            inf.boardName = Tools::fromStd(o.at("boardName").str());
            inf.expires = QDateTime::fromString(Tools::fromStd(o.at("expires").str()), "dd.MM.yyyy:hh");
            inf.level = int(o.at("level").number());
            inf.reason = Tools::fromStd(o.at("reason").str());
            bans << inf;
        }
        QString err;
        if (!Database::banUser(server.request(), ip, bans, &err)) {
            server.return_error(Tools::toStd(err));
            Tools::log(server, "ajax_ban_user", "fail:" + err, logTarget);
            return;
        }
        server.return_result(true);
        Tools::log(server, "ajax_ban_user", "success", logTarget);
    } catch (const cppcms::json::bad_value_cast &e) {
        QString err = Tools::fromStd(e.what());
        server.return_error(Tools::toStd(err));
        Tools::log(server, "ajax_ban_user", "fail:" + err);
    } catch (const std::out_of_range &e) {
        QString err = Tools::fromStd(e.what());
        server.return_error(Tools::toStd(err));
        Tools::log(server, "ajax_ban_user", "fail:" + err);
    } catch (const std::exception &e) {
        QString err = Tools::fromStd(e.what());
        server.return_error(Tools::toStd(err));
        Tools::log(server, "ajax_ban_user", "fail:" + err);
    }
}

void ActionAjaxHandler::delall(std::string userIp, std::string boardName)
{
    try {
        QString ip = Tools::fromStd(userIp);
        QString board = Tools::fromStd(boardName);
        QString logTarget = ip + "/" + board;
        Tools::log(server, "ajax_delall", "begin", logTarget);
        QString err;
        if (!Database::delall(server.request(), ip, board, &err)) {
            server.return_error(Tools::toStd(err));
            Tools::log(server, "ajax_delall", "fail:" + err, logTarget);
            return;
        }
        server.return_result(true);
        Tools::log(server, "ajax_delall", "success", logTarget);
    } catch (const cppcms::json::bad_value_cast &e) {
        QString err = Tools::fromStd(e.what());
        server.return_error(Tools::toStd(err));
        Tools::log(server, "ajax_delall", "fail:" + err);
    } catch (const std::out_of_range &e) {
        QString err = Tools::fromStd(e.what());
        server.return_error(Tools::toStd(err));
        Tools::log(server, "ajax_delall", "fail:" + err);
    } catch (const std::exception &e) {
        QString err = Tools::fromStd(e.what());
        server.return_error(Tools::toStd(err));
        Tools::log(server, "ajax_delall", "fail:" + err);
    }
}

void ActionAjaxHandler::deleteFile(std::string boardName, std::string fileName, std::string password)
{
    try {
        QString bn = Tools::fromStd(boardName);
        QString fn = Tools::fromStd(fileName);
        QString logTarget = bn + "/" + fn;
        Tools::log(server, "ajax_delete_file", "begin", logTarget);
        if (!testBan(bn))
            return Tools::log(server, "ajax_delete_file", "fail:ban", logTarget);
        QString err;
        if (!Database::deleteFile(bn, fn, server.request(), Tools::toHashpass(Tools::fromStd(password)), &err)) {
            server.return_error(Tools::toStd(err));
            Tools::log(server, "ajax_delete_file", "fail:" + err, logTarget);
            return;
        }
        server.return_result(true);
        Tools::log(server, "ajax_delete_file", "success", logTarget);
    } catch (const std::exception &e) {
        QString err = Tools::fromStd(e.what());
        server.return_error(Tools::toStd(err));
        Tools::log(server, "ajax_delete_file", "fail:" + err);
    }
}

void ActionAjaxHandler::deletePost(std::string boardName, long long postNumber, std::string password)
{
    try {
        QString bn = Tools::fromStd(boardName);
        quint64 pn = postNumber > 0 ? quint64(postNumber) : 0;
        QString logTarget = bn + "/" + QString::number(pn);
        Tools::log(server, "ajax_delete_post", "begin", logTarget);
        if (!testBan(bn))
            return Tools::log(server, "ajax_delete_post", "fail:ban", logTarget);
        QString err;
        if (!Database::deletePost(bn, pn, server.request(), Tools::toHashpass(Tools::fromStd(password)), &err)) {
            server.return_error(Tools::toStd(err));
            Tools::log(server, "ajax_delete_post", "fail:" + err, logTarget);
            return;
        }
        server.return_result(true);
        Tools::log(server, "ajax_delete_post", "success", logTarget);
    } catch (const std::exception &e) {
        QString err = Tools::fromStd(e.what());
        server.return_error(Tools::toStd(err));
        Tools::log(server, "ajax_delete_post", "fail:" + err);
    }
}

void ActionAjaxHandler::editAudioTags(std::string boardName, std::string fileName, std::string password,
                                      const cppcms::json::object &tags)
{
    try {
        QString bn = Tools::fromStd(boardName);
        QString fn = Tools::fromStd(fileName);
        QString logTarget = bn + "/" + fn;
        Tools::log(server, "ajax_edit_audio_tags", "begin", logTarget);
        if (!testBan(bn))
            return Tools::log(server, "ajax_edit_audio_tags", "fail:ban", logTarget);
        QByteArray pwd = Tools::toHashpass(Tools::fromStd(password));
        QVariantMap m = Tools::fromJson(tags).toMap();
        QString err;
        if (!Database::editAudioTags(bn, fn, server.request(), pwd, m, &err)) {
            server.return_error(Tools::toStd(err));
            Tools::log(server, "ajax_edit_audio_tags", "fail:" + err, logTarget);
            return;
        }
        server.return_result(true);
        Tools::log(server, "ajax_edit_audio_tags", "success", logTarget);
    } catch (const std::exception &e) {
        QString err = Tools::fromStd(e.what());
        server.return_error(Tools::toStd(err));
        Tools::log(server, "ajax_edit_audio_tags", "fail:" + err);
    }
}

void ActionAjaxHandler::editPost(const cppcms::json::object &params)
{
    try {
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
        std::string mm = params.at("markupMode").str();
        QString mmq = Tools::fromStd(mm);
        p.extendedWakabaMarkEnabled = mmq.contains("ewm", Qt::CaseInsensitive);
        p.bbCodeEnabled = mmq.contains("bbc", Qt::CaseInsensitive);
        p.userData = params.at("userData");
        QString err;
        p.error = &err;
        if (!Database::editPost(p)) {
            server.return_error(Tools::toStd(err));
            Tools::log(server, "ajax_edit_post", "fail:" + err, logTarget);
            return;
        }

        server.response().set_cookie(cppcms::http::cookie("markupMode", mm, UINT_MAX, "/"));
        cppcms::json::object refs;
        foreach (const Database::RefKey &key, p.referencedPosts.keys()) {
            std::string k = Tools::toStd(key.boardName) + "/" + Tools::toStd(QString::number(key.postNumber));
            refs[k] = Tools::toStd(QString::number(p.referencedPosts.value(key)));
        }
        server.return_result(refs);
        Tools::log(server, "ajax_edit_post", "success", logTarget);
    } catch (const cppcms::json::bad_value_cast &e) {
        QString err = Tools::fromStd(e.what());
        server.return_error(Tools::toStd(err));
        Tools::log(server, "ajax_edit_post", "fail:" + err);
    } catch (const std::out_of_range &e) {
        QString err = Tools::fromStd(e.what());
        server.return_error(Tools::toStd(err));
        Tools::log(server, "ajax_edit_post", "fail:" + err);
    } catch (const std::exception &e) {
        QString err = Tools::fromStd(e.what());
        server.return_error(Tools::toStd(err));
        Tools::log(server, "ajax_edit_post", "fail:" + err);
    }
}

void ActionAjaxHandler::getBoards()
{
    try {
        Tools::log(server, "ajax_get_boards", "begin");
        AbstractBoard::BoardInfoList list = AbstractBoard::boardInfos(Tools::locale(server.request()), false);
        cppcms::json::array arr;
        foreach (const AbstractBoard::BoardInfo &inf, list) {
            cppcms::json::object o;
            o["name"] = inf.name;
            o["title"] = inf.title;
            arr.push_back(o);
        }
        server.return_result(arr);
        Tools::log(server, "ajax_get_boards", "success");
    } catch (const std::exception &e) {
        QString err = Tools::fromStd(e.what());
        server.return_error(Tools::toStd(err));
        Tools::log(server, "ajax_get_boards", "fail:" + err);
    }
}

void ActionAjaxHandler::getCaptchaQuota(std::string boardName)
{
    try {
        QString bn = Tools::fromStd(boardName);
        QString logTarget = bn;
        Tools::log(server, "ajax_get_captcha_quota", "begin", logTarget);
        if (!testBan(bn, true))
            return Tools::log(server, "ajax_get_captcha_quota", "fail:ban", logTarget);
        AbstractBoard::LockingWrapper board = AbstractBoard::board(bn);
        TranslatorStd ts(server.request());
        if (board.isNull()) {
            std::string err = ts.translate("ActionAjaxHandler", "No such board", "error");
            server.return_error(err);
            Tools::log(server, "ajax_get_captcha_quota", "fail:" + Tools::fromStd(err), logTarget);
            return;
        }
        server.return_result(board->captchaQuota(server.request()));
        Tools::log(server, "ajax_get_captcha_quota", "success", logTarget);
    } catch (const std::exception &e) {
        QString err = Tools::fromStd(e.what());
        server.return_error(Tools::toStd(err));
        Tools::log(server, "ajax_get_captcha_quota", "fail:" + err);
    }
}

void ActionAjaxHandler::getCoubVideoInfo(std::string videoId)
{
    try {
        QString id = Tools::fromStd(videoId);
        QString logTarget = id;
        Tools::log(server, "ajax_get_coub_video_info", "begin", logTarget);
        std::string result;
        try {
            curlpp::Cleanup curlppCleanup;
            Q_UNUSED(curlppCleanup)
            QString url = "https://coub.com/api/oembed.json?url=coub.com/view/" + id;
            curlpp::Easy request;
            request.setOpt(curlpp::options::Url(Tools::toStd(url)));
            std::ostringstream os;
            os << request;
            result = os.str();
        } catch (curlpp::RuntimeError &e) {
            QString err = Tools::fromStd(e.what());
            server.return_error(Tools::toStd(err));
            Tools::log(server, "ajax_get_coub_video_info", "fail:" + err);
            return;
        } catch(curlpp::LogicError &e) {
            QString err = Tools::fromStd(e.what());
            server.return_error(Tools::toStd(err));
            Tools::log(server, "ajax_get_coub_video_info", "fail:" + err);
            return;
        }
        std::istringstream is(result);
        TranslatorStd ts(server.request());
        cppcms::json::value v;
        if (!v.load(is, true)) {
            std::string err = ts.translate("ActionAjaxHandler", "Internal error", "error");
            server.return_error(err);
            Tools::log(server, "ajax_get_coub_video_info", "fail:" + Tools::fromStd(err), logTarget);
            return;
        }
        cppcms::json::object o = v.object();
        server.return_result(o);
        Tools::log(server, "ajax_get_coub_video_info", "success", logTarget);
    } catch (const std::exception &e) {
        QString err = Tools::fromStd(e.what());
        server.return_error(Tools::toStd(err));
        Tools::log(server, "ajax_get_coub_video_info", "fail:" + err);
    }
}

void ActionAjaxHandler::getFileExistence(std::string boardName, std::string hash)
{
    try {
        QString bn = Tools::fromStd(boardName);
        QString h = Tools::fromStd(hash);
        QString logTarget = bn + "/" + h;
        Tools::log(server, "ajax_get_file_existence", "begin", logTarget);
        if (!testBan(bn, true))
            return Tools::log(server, "ajax_get_file_existence", "fail:ban", logTarget);
        bool ok = false;
        bool exists = Database::fileExists(h, &ok);
        TranslatorStd ts(server.request());
        if (!ok) {
            std::string err = ts.translate("ActionAjaxHandler", "Internal database error", "error");
            server.return_error(err);
            Tools::log(server, "ajax_get_file_existence", "fail:" + Tools::fromStd(err), logTarget);
            return;
        }
        server.return_result(exists);
        Tools::log(server, "ajax_get_file_existence", "success", logTarget);
    } catch (const std::exception &e) {
        QString err = Tools::fromStd(e.what());
        server.return_error(Tools::toStd(err));
        Tools::log(server, "ajax_get_file_existence", "fail:" + err);
    }
}

void ActionAjaxHandler::getFileMetaData(std::string boardName, std::string fileName)
{
    try {
        QString bn = Tools::fromStd(boardName);
        QString fn = Tools::fromStd(fileName);
        QString logTarget = bn + "/" + fn;
        Tools::log(server, "ajax_get_file_meta_data", "begin", logTarget);
        if (!testBan(bn, true))
            return Tools::log(server, "ajax_get_file_meta_data", "fail:ban", logTarget);
        bool ok = false;
        QString err;
        TranslatorStd ts(server.request());
        QVariant md = Database::getFileMetaData(fn, &ok, &err, ts.locale());
        if (!ok) {
            std::string err = ts.translate("ActionAjaxHandler", "Internal database error", "error");
            server.return_error(err);
            Tools::log(server, "ajax_get_file_meta_data", "fail:" + Tools::fromStd(err), logTarget);
            return;
        }
        server.return_result(Tools::toJson(md));
        Tools::log(server, "ajax_get_file_meta_data", "success", logTarget);
    } catch (const std::exception &e) {
        QString err = Tools::fromStd(e.what());
        server.return_error(Tools::toStd(err));
        Tools::log(server, "ajax_get_file_meta_data", "fail:" + err);
    }
}

void ActionAjaxHandler::getNewPostCount(std::string boardName, long long lastPostNumber)
{
    try {
        QString bn = Tools::fromStd(boardName);
        quint64 lpn = lastPostNumber > 0 ? quint64(lastPostNumber) : 0;
        QString logTarget = bn + "/" + QString::number(lpn);
        Tools::log(server, "ajax_get_new_post_count", "begin", logTarget);
        AbstractBoard::LockingWrapper board = AbstractBoard::board(bn);
        if (board.isNull()) {
            TranslatorQt tq(server.request());
            QString err = tq.translate("ActionAjaxHandler", "No such board", "error");
            Tools::log(server, "ajax_get_new_post_count", "fail:" + err, logTarget);
            server.return_error(Tools::toStd(err));
        }
        if (!testBan(bn, true))
            return Tools::log(server, "ajax_get_new_post_count", "fail:ban", logTarget);
        bool ok = false;
        QString err;
        int count = Database::getNewPostCount(server.request(), bn, lpn, &ok, &err);
        if (!ok) {
            server.return_error(Tools::toStd(err));
            Tools::log(server, "ajax_get_new_post_count", "fail:" + err, logTarget);
            return;
        }
        server.return_result(count);
        Tools::log(server, "ajax_get_new_post_count", "success", logTarget);
    } catch (const std::exception &e) {
        QString err = Tools::fromStd(e.what());
        server.return_error(Tools::toStd(err));
        Tools::log(server, "ajax_get_new_post_count", "fail:" + err);
    }
}

void ActionAjaxHandler::getNewPostCountEx(const cppcms::json::object &numbers)
{
    try {
        Tools::log(server, "ajax_get_new_post_count_ex", "begin");
        QVariantMap m;
        for (cppcms::json::object::const_iterator i = numbers.begin(); i != numbers.end(); ++i) {
            QString bn = Tools::fromStd(i->first);
            quint64 lpn = i->second.number() > 0 ? quint64(i->second.number()) : 0;
            if (!testBan(bn, true))
                continue;
            m.insert(bn, lpn);
        }
        bool ok = false;
        QString err;
        QVariantMap r = Database::getNewPostCountEx(server.request(), m, &ok, &err);
        if (!ok) {
            server.return_error(Tools::toStd(err));
            Tools::log(server, "ajax_get_new_post_count_ex", "fail:" + err);
            return;
        }
        server.return_result(Tools::toJson(r));
        Tools::log(server, "ajax_get_new_post_count_ex", "success");
    } catch (const std::exception &e) {
        QString err = Tools::fromStd(e.what());
        server.return_error(Tools::toStd(err));
        Tools::log(server, "ajax_get_new_post_count_ex", "fail:" + err);
    }
}

void ActionAjaxHandler::getNewPosts(std::string boardName, long long threadNumber, long long lastPostNumber)
{
    try {
        QString bn = Tools::fromStd(boardName);
        quint64 tn = threadNumber > 0 ? quint64(threadNumber) : 0;
        quint64 lpn = lastPostNumber > 0 ? quint64(lastPostNumber) : 0;
        QString logTarget = bn + "/" + QString::number(tn) + "/" + QString::number(lpn);
        Tools::log(server, "ajax_get_new_posts", "begin", logTarget);
        AbstractBoard::LockingWrapper board = AbstractBoard::board(bn);
        if (board.isNull()) {
            TranslatorQt tq(server.request());
            QString err = tq.translate("ActionAjaxHandler", "No such board", "error");
            Tools::log(server, "ajax_get_new_posts", "fail:" + err, logTarget);
            server.return_error(Tools::toStd(err));
        }
        if (!testBan(bn, true))
            return Tools::log(server, "ajax_get_new_posts", "fail:ban", logTarget);
        bool ok = false;
        QString err;
        const cppcms::http::request &req = server.request();
        QList<Content::Post> posts = Database::getNewPostsC(req, bn, tn, lpn, &ok, &err);
        if (!ok) {
            server.return_error(Tools::toStd(err));
            Tools::log(server, "ajax_get_new_posts", "fail:" + err, logTarget);
            return;
        }
        cppcms::json::array a;
        foreach (const Content::Post &p, posts)
            a.push_back(board->toJson(p, server.request()));
        server.return_result(a);
        Tools::log(server, "ajax_get_new_posts", "success", logTarget);
    } catch (const std::exception &e) {
        QString err = Tools::fromStd(e.what());
        server.return_error(Tools::toStd(err));
        Tools::log(server, "ajax_get_new_posts", "fail:" + err);
    }
}

void ActionAjaxHandler::getPost(std::string boardName, long long postNumber)
{
    try {
        QString bn = Tools::fromStd(boardName);
        quint64 pn = postNumber > 0 ? quint64(postNumber) : 0;
        QString logTarget = bn + "/" + QString::number(pn);
        Tools::log(server, "ajax_get_post", "begin", logTarget);
        AbstractBoard::LockingWrapper board = AbstractBoard::board(bn);
        if (board.isNull()) {
            TranslatorQt tq(server.request());
            QString err = tq.translate("ActionAjaxHandler", "No such board", "error");
            Tools::log(server, "ajax_post", "fail:" + err, logTarget);
            server.return_error(Tools::toStd(err));
        }
        if (!testBan(Tools::fromStd(boardName), true))
            return Tools::log(server, "ajax_get_post", "fail:ban", logTarget);
        bool ok = false;
        QString err;
        const cppcms::http::request &req = server.request();
        Content::Post post = Database::getPostC(req, bn, pn, &ok, &err);
        if (!ok) {
            server.return_error(Tools::toStd(err));
            Tools::log(server, "ajax_get_post", "fail:" + err, logTarget);
            return;
        }
        server.return_result(board->toJson(post, server.request()));
        Tools::log(server, "ajax_get_post", "success", logTarget);
    } catch (const std::exception &e) {
        QString err = Tools::fromStd(e.what());
        server.return_error(Tools::toStd(err));
        Tools::log(server, "ajax_get_post", "fail:" + err);
    }
}

void ActionAjaxHandler::getThreadNumbers(std::string boardName)
{
    try {
        QString bn = Tools::fromStd(boardName);
        QString logTarget = bn;
        Tools::log(server, "ajax_get_thread_numbers", "begin", logTarget);
        if (!testBan(bn, true))
            return Tools::log(server, "ajax_get_thread_numbers", "fail:ban", logTarget);
        bool ok = false;
        QString err;
        QList<quint64> list = Database::getThreadNumbers(server.request(), bn, &ok, &err);
        if (!ok) {
            server.return_error(Tools::toStd(err));
            Tools::log(server, "ajax_get_thread_numbers", "fail:" + err, logTarget);
            return;
        }
        cppcms::json::array arr;
        foreach (quint64 pn, list)
            arr.push_back(pn);
        server.return_result(arr);
        Tools::log(server, "ajax_get_thread_numbers", "success", logTarget);
    } catch (const std::exception &e) {
        QString err = Tools::fromStd(e.what());
        server.return_error(Tools::toStd(err));
        Tools::log(server, "ajax_get_thread_numbers", "fail:" + err);
    }
}

void ActionAjaxHandler::getUserBanInfo(std::string userIp)
{
    try {
        QString ip = Tools::fromStd(userIp);
        QString logTarget = ip;
        Tools::log(server, "ajax_get_user_ban_info", "begin", logTarget);
        bool ok = false;
        QString err;
        QMap<QString, Database::BanInfo> map = Database::userBanInfo(ip, &ok, &err, Tools::locale(server.request()));
        if (!ok) {
            server.return_error(Tools::toStd(err));
            Tools::log(server, "ajax_get_user_ban_info", "fail:" + err, logTarget);
            return;
        }
        cppcms::json::object o;
        foreach (const Database::BanInfo &inf, map.values()) {
            cppcms::json::object oo;
            oo["boardName"] = Tools::toStd(inf.boardName);
            oo["dateTime"] = Tools::toStd(Tools::dateTime(inf.dateTime,
                                                          server.request()).toString("dd.MM.yyyy:hh:mm:ss"));
            oo["expires"] = Tools::toStd(Tools::dateTime(inf.expires, server.request()).toString("dd.MM.yyyy:hh"));
            oo["level"] = inf.level;
            oo["reason"] = Tools::toStd(inf.reason);
            o[Tools::toStd(inf.boardName)] = oo;
        }
        server.return_result(o);
        Tools::log(server, "ajax_get_user_ban_info", "success", logTarget);
    } catch (const std::exception &e) {
        QString err = Tools::fromStd(e.what());
        server.return_error(Tools::toStd(err));
        Tools::log(server, "ajax_get_user_ban_info", "fail:" + err);
    }
}

void ActionAjaxHandler::getYandexCaptchaImage(std::string type)
{
    try {
        QString t = Tools::fromStd(type);
        QString logTarget = t;
        Tools::log(server, "ajax_get_yandex_captcha_image", "begin", logTarget);
        TranslatorQt tq(server.request());
        if ("elatm" != t && "estd" != t && "rus" != t) {
            QString err = tq.translate("ActionAjaxHandler", "Invalid captcha type", "error");
            Tools::log(server, "ajax_get_yandex_captcha_image", "fail:" + err, logTarget);
            server.return_error(Tools::toStd(err));
            return;
        }
        bool ok = false;
        QString err;
        AbstractYandexCaptchaEngine::CaptchaInfo inf = AbstractYandexCaptchaEngine::getCaptchaInfo(t, tq.locale(), &ok,
                                                                                                   &err);
        if (!ok) {
            Tools::log(server, "ajax_get_yandex_captcha_image", "fail:" + err, logTarget);
            server.return_error(Tools::toStd(err));
            return;
        }
        cppcms::json::object o;
        o["challenge"] = Tools::toStd(inf.challenge);
        o["url"] = Tools::toStd(inf.url);
        server.return_result(o);
        Tools::log(server, "ajax_get_yandex_captcha_image", "success", logTarget);
    } catch (const std::exception &e) {
        QString err = Tools::fromStd(e.what());
        server.return_error(Tools::toStd(err));
        Tools::log(server, "ajax_get_yandex_captcha_image", "fail:" + err);
    }
}

QList<ActionAjaxHandler::Handler> ActionAjaxHandler::handlers() const
{
    QList<ActionAjaxHandler::Handler> list;
    ActionAjaxHandler *self = const_cast<ActionAjaxHandler *>(this);
    list << Handler("ban_poster", cppcms::rpc::json_method(&ActionAjaxHandler::banPoster, self), method_role);
    list << Handler("ban_user", cppcms::rpc::json_method(&ActionAjaxHandler::banUser, self), method_role);
    list << Handler("delall", cppcms::rpc::json_method(&ActionAjaxHandler::delall, self), method_role);
    list << Handler("delete_file", cppcms::rpc::json_method(&ActionAjaxHandler::deleteFile, self), method_role);
    list << Handler("delete_post", cppcms::rpc::json_method(&ActionAjaxHandler::deletePost, self), method_role);
    list << Handler("edit_audio_tags", cppcms::rpc::json_method(&ActionAjaxHandler::editAudioTags, self), method_role);
    list << Handler("edit_post", cppcms::rpc::json_method(&ActionAjaxHandler::editPost, self), method_role);
    list << Handler("get_boards", cppcms::rpc::json_method(&ActionAjaxHandler::getBoards, self), method_role);
    list << Handler("get_captcha_quota", cppcms::rpc::json_method(&ActionAjaxHandler::getCaptchaQuota, self),
                    method_role);
    list << Handler("get_coub_video_info", cppcms::rpc::json_method(&ActionAjaxHandler::getCoubVideoInfo, self),
                    method_role);
    list << Handler("get_file_existence", cppcms::rpc::json_method(&ActionAjaxHandler::getFileExistence, self),
                    method_role);
    list << Handler("get_file_meta_data", cppcms::rpc::json_method(&ActionAjaxHandler::getFileMetaData, self),
                    method_role);
    list << Handler("get_new_post_count", cppcms::rpc::json_method(&ActionAjaxHandler::getNewPostCount, self),
                    method_role);
    list << Handler("get_new_post_count_ex", cppcms::rpc::json_method(&ActionAjaxHandler::getNewPostCountEx, self),
                    method_role);
    list << Handler("get_new_posts", cppcms::rpc::json_method(&ActionAjaxHandler::getNewPosts, self), method_role);
    list << Handler("get_post", cppcms::rpc::json_method(&ActionAjaxHandler::getPost, self), method_role);
    list << Handler("get_thread_numbers", cppcms::rpc::json_method(&ActionAjaxHandler::getThreadNumbers, self),
                    method_role);
    list << Handler("get_user_ban_info", cppcms::rpc::json_method(&ActionAjaxHandler::getUserBanInfo, self),
                    method_role);
    list << Handler("get_yandex_captcha_image",
                    cppcms::rpc::json_method(&ActionAjaxHandler::getYandexCaptchaImage, self), method_role);
    list << Handler("move_thread", cppcms::rpc::json_method(&ActionAjaxHandler::moveThread, self), method_role);
    list << Handler("set_thread_fixed", cppcms::rpc::json_method(&ActionAjaxHandler::setThreadFixed, self),
                    method_role);
    list << Handler("set_thread_opened", cppcms::rpc::json_method(&ActionAjaxHandler::setThreadOpened, self),
                    method_role);
    list << Handler("set_vote_opened", cppcms::rpc::json_method(&ActionAjaxHandler::setVoteOpened, self), method_role);
    list << Handler("unvote", cppcms::rpc::json_method(&ActionAjaxHandler::unvote, self), method_role);
    list << Handler("vote", cppcms::rpc::json_method(&ActionAjaxHandler::vote, self), method_role);
    return list;
}

void ActionAjaxHandler::moveThread(std::string sourceBoardName, long long threadNumber, std::string targetBoardName)
{
    try {
        QString sbn = Tools::fromStd(sourceBoardName);
        quint64 tn = threadNumber > 0 ? quint64(threadNumber) : 0;
        QString tbn = Tools::fromStd(targetBoardName);
        QString logTarget = sbn + "/" + QString::number(tn) + "/" + tbn;
        Tools::log(server, "ajax_move_thread", "begin", logTarget);
        if (!testBan(sbn) || !testBan(tbn))
            return Tools::log(server, "ajax_move_thread", "fail:ban", logTarget);
        QString err;
        quint64 ntn = Database::moveThread(server.request(), sbn, tn, tbn, &err);
        if (!ntn) {
            server.return_error(Tools::toStd(err));
            Tools::log(server, "ajax_move_thread", "fail:" + err, logTarget);
            return;
        }
        server.return_result(ntn);
        Tools::log(server, "ajax_move_thread", "success", logTarget);
    } catch (const std::exception &e) {
        QString err = Tools::fromStd(e.what());
        server.return_error(Tools::toStd(err));
        Tools::log(server, "ajax_move_thread", "fail:" + err);
    }
}

void ActionAjaxHandler::setThreadFixed(std::string boardName, long long threadNumber, bool fixed)
{
    try {
        QString bn = Tools::fromStd(boardName);
        quint64 tn = threadNumber > 0 ? quint64(threadNumber) : 0;
        QString logTarget = bn + "/" + QString::number(tn) + "/" + QString(fixed ? "true" : "false");
        Tools::log(server, "ajax_set_thread_fixed", "begin", logTarget);
        if (!testBan(bn))
            return Tools::log(server, "ajax_set_thread_fixed", "fail:ban", logTarget);
        QString err;
        if (!Database::setThreadFixed(bn, tn, fixed, server.request(), &err)) {
            server.return_error(Tools::toStd(err));
            Tools::log(server, "ajax_set_thread_fixed", "fail:" + err, logTarget);
            return;
        }
        server.return_result(true);
        Tools::log(server, "ajax_set_thread_fixed", "success", logTarget);
    } catch (const std::exception &e) {
        QString err = Tools::fromStd(e.what());
        server.return_error(Tools::toStd(err));
        Tools::log(server, "ajax_set_thread_fixed", "fail:" + err);
    }
}

void ActionAjaxHandler::setThreadOpened(std::string boardName, long long threadNumber, bool opened)
{
    try {
        QString bn = Tools::fromStd(boardName);
        quint64 tn = threadNumber > 0 ? quint64(threadNumber) : 0;
        QString logTarget = bn + "/" + QString::number(tn) + "/" + QString(opened ? "true" : "false");
        Tools::log(server, "ajax_set_thread_opened", "begin", logTarget);
        if (!testBan(bn))
            return Tools::log(server, "ajax_set_thread_opened", "fail:ban", logTarget);
        QString err;
        if (!Database::setThreadOpened(bn, tn, opened, server.request(), &err)) {
            server.return_error(Tools::toStd(err));
            Tools::log(server, "ajax_set_thread_opened", "fail:" + err, logTarget);
            return;
        }
        server.return_result(true);
        Tools::log(server, "ajax_set_thread_opened", "success", logTarget);
    } catch (const std::exception &e) {
        QString err = Tools::fromStd(e.what());
        server.return_error(Tools::toStd(err));
        Tools::log(server, "ajax_set_thread_opened", "fail:" + err);
    }
}

void ActionAjaxHandler::setVoteOpened(long long postNumber, bool opened, std::string password)
{
    try {
        quint64 pn = postNumber > 0 ? quint64(postNumber) : 0;
        QString logTarget = QString::number(pn) + "/" + QString(opened ? "true" : "false");
        Tools::log(server, "ajax_set_vote_opened", "begin", logTarget);
        if (!testBan("rpg"))
            return Tools::log(server, "ajax_set_vote_opened", "fail:ban", logTarget);
        QString err;
        QByteArray pwd = Tools::toHashpass(Tools::fromStd(password));
        if (!Database::setVoteOpened(pn, opened, pwd, server.request(), &err)) {
            server.return_error(Tools::toStd(err));
            Tools::log(server, "ajax_set_vote_opened", "fail:" + err, logTarget);
            return;
        }
        server.return_result(true);
        Tools::log(server, "ajax_set_vote_opened", "success", logTarget);
    } catch (const std::exception &e) {
        QString err = Tools::fromStd(e.what());
        server.return_error(Tools::toStd(err));
        Tools::log(server, "ajax_set_vote_opened", "fail:" + err);
    }
}

void ActionAjaxHandler::unvote(long long postNumber)
{
    try {
        quint64 pn = postNumber > 0 ? quint64(postNumber) : 0;
        QString logTarget = QString::number(pn);
        Tools::log(server, "ajax_unvote", "begin", logTarget);
        if (!testBan("rpg"))
            return Tools::log(server, "ajax_unvote", "fail:ban", logTarget);
        QString err;
        if (!Database::unvote(pn, server.request(), &err)) {
            server.return_error(Tools::toStd(err));
            Tools::log(server, "ajax_unvote", "fail:" + err, logTarget);
            return;
        }
        server.return_result(true);
        Tools::log(server, "ajax_unvote", "success", logTarget);
    } catch (const std::exception &e) {
        QString err = Tools::fromStd(e.what());
        server.return_error(Tools::toStd(err));
        Tools::log(server, "ajax_unvote", "fail:" + err);
    }
}

void ActionAjaxHandler::vote(long long postNumber, const cppcms::json::array &votes)
{
    try {
        quint64 pn = postNumber > 0 ? quint64(postNumber) : 0;
        QString logTarget = QString::number(pn);
        Tools::log(server, "ajax_vote", "begin", logTarget);
        if (!testBan("rpg"))
            return Tools::log(server, "ajax_vote", "fail:ban", logTarget);
        QString err;
        QStringList list;
        foreach (int i, bRangeD(0, votes.size() - 1))
            list << Tools::fromStd(votes.at(i).str());
        if (!Database::vote(pn, list, server.request(), &err)) {
            server.return_error(Tools::toStd(err));
            Tools::log(server, "ajax_vote", "fail:" + err, logTarget);
            return;
        }
        server.return_result(true);
        Tools::log(server, "ajax_vote", "success", logTarget);
    } catch (const cppcms::json::bad_value_cast &e) {
        QString err = Tools::fromStd(e.what());
        server.return_error(Tools::toStd(err));
        Tools::log(server, "ajax_vote", "fail:" + err);
    } catch (const std::exception &e) {
        QString err = Tools::fromStd(e.what());
        server.return_error(Tools::toStd(err));
        Tools::log(server, "ajax_vote", "fail:" + err);
    }
}
