#include "actionroute.h"

#include "board/abstractboard.h"
#include "controller.h"
#include "database.h"
#include "settingslocker.h"
#include "tools.h"
#include "translator.h"

#include <BeQtGlobal>

#include <QByteArray>
#include <QCryptographicHash>
#include <QDebug>
#include <QRegExp>
#include <QSettings>
#include <QString>
#include <QStringList>
#include <QVariant>

#include <cppcms/application.h>
#include <cppcms/http_cookie.h>
#include <cppcms/http_request.h>
#include <cppcms/http_response.h>

#include <climits>
#include <string>

ActionRoute::ActionRoute(cppcms::application &app) :
    AbstractRoute(app)
{
    //
}

QStringList ActionRoute::availableActions()
{
    return actionMap().keys();
}

void ActionRoute::handle(std::string action)
{
    typedef QMap<QString, double> WeightMap;
    init_once(WeightMap, weightMap, WeightMap()) {
        weightMap.insert("add_file", 450.0);
        weightMap.insert("ban_poster", 25.0);
        weightMap.insert("ban_user", 40.0);
        weightMap.insert("change_locale", 0.1);
        weightMap.insert("change_settings", 0.3);
        weightMap.insert("create_post", 400.0);
        weightMap.insert("create_thread", 2000.0);
        weightMap.insert("delall", 7000.0);
        weightMap.insert("delete_file", 50.0);
        weightMap.insert("delete_post", 500.0);
        weightMap.insert("edit_audio_tags", 25.0);
        weightMap.insert("edit_post", 50.0);
        weightMap.insert("login", 0.1);
        weightMap.insert("logout", 0.1);
        weightMap.insert("move_thread", 3000.0);
        weightMap.insert("set_thread_fixed", 10.0);
        weightMap.insert("set_thread_opened", 50.0);
        weightMap.insert("set_vote_opened", 50.0);
        weightMap.insert("unvote", 50.0);
        weightMap.insert("vote", 30.0);
    }
    QString a = Tools::fromStd(action);
    double weight = weightMap.value(a);
    if (!qFuzzyIsNull(weight))
        DDOS_A(weight);
    Tools::PostParameters params = Tools::postParameters(application.request());
    QString logTarget = params.value("board");
    Tools::log(application, "action/" + a, "begin", logTarget);
    QString err;
    if (!Controller::testRequest(application, Controller::PostRequest, &err)) {
        Tools::log(application, "action/" + a, "fail:" + err, logTarget);
        DDOS_POST_A
        return;
    }
    TranslatorQt tq(application.request());
    HandleActionMap map = actionMap();
    if (!map.contains(a)) {
        err = tq.translate("ActionRoute", "Unknown action", "error");
        Controller::renderError(application, err,
                                tq.translate("ActionRoute", "There is no such action", "description"));
        Tools::log(application, "action/" + a, "fail:" + err, logTarget);
        DDOS_POST_A
        return;
    }
    (this->*map.value(a))(a, params, tq);
    DDOS_POST_A
}

unsigned int ActionRoute::handlerArgumentCount() const
{
    return 1;
}

std::string ActionRoute::key() const
{
    return "action";
}

int ActionRoute::priority() const
{
    return 0;
}

std::string ActionRoute::regex() const
{
    static const QString actionRx = "(" + availableActions().join("|") + ")";
    return Tools::toStd("/action/" + actionRx);
}

std::string ActionRoute::url() const
{
    return "/action/{1}";
}

ActionRoute::HandleActionMap ActionRoute::actionMap()
{
    init_once(HandleActionMap, map, HandleActionMap()) {
        map.insert("add_file", &ActionRoute::handleAddFile);
        map.insert("ban_poster", &ActionRoute::handleBanPoster);
        map.insert("ban_user", &ActionRoute::handleBanUser);
        map.insert("change_locale", &ActionRoute::handleChangeLocale);
        map.insert("change_settings", &ActionRoute::handleChangeSettings);
        map.insert("create_post", &ActionRoute::handleCreatePost);
        map.insert("create_thread", &ActionRoute::handleCreateThread);
        map.insert("delall", &ActionRoute::handleDelall);
        map.insert("delete_file", &ActionRoute::handleDeleteFile);
        map.insert("delete_post", &ActionRoute::handleDeletePost);
        map.insert("edit_audio_tags", &ActionRoute::handleEditAudioTags);
        map.insert("edit_post", &ActionRoute::handleEditPost);
        map.insert("login", &ActionRoute::handleLogin);
        map.insert("logout", &ActionRoute::handleLogout);
        map.insert("move_thread", &ActionRoute::handleMoveThread);
        map.insert("set_thread_fixed", &ActionRoute::handleSetThreadFixed);
        map.insert("set_thread_opened", &ActionRoute::handleSetThreadOpened);
        map.insert("set_vote_opened", &ActionRoute::handleSetVoteOpened);
        map.insert("unvote", &ActionRoute::handleUnvote);
        map.insert("vote", &ActionRoute::handleVote);
    }
    return map;
}

void ActionRoute::handleAddFile(const QString &action, const Tools::PostParameters &params, const Translator::Qt &tq)
{
    AbstractBoard::LockingWrapper board = AbstractBoard::board(params.value("board"));
    if (!testBoard(board.data(), action, params.value("board"), tq))
        return;
    board->addFile(application);
}

void ActionRoute::handleBanPoster(const QString &action, const Tools::PostParameters &params, const Translator::Qt &tq)
{
    QString sourceBoard = params.value("boardName");
    quint64 postNumber = params.value("postNumber").toULongLong();
    QString logTarget = sourceBoard + "/" + QString::number(postNumber);
    if (!Controller::testBanNonAjax(application, Controller::WriteAction, sourceBoard))
        return Tools::log(application, "action/" + action, "fail:ban", logTarget);
    QList<Database::BanInfo> bans;
    foreach (const QString &key, params.keys()) {
        if (!key.startsWith("ban_board_", Qt::CaseInsensitive))
            continue;
        Database::BanInfo inf;
        inf.boardName = params.value(key);
        inf.expires = QDateTime::fromString(params.value("ban_expires" + inf.boardName), "dd.MM.yyyy:hh");
        inf.level = params.value("ban_level_" + inf.boardName).toInt();
        inf.reason = params.value("ban_reason_" + inf.boardName);
        bans << inf;
    }
    QString err;
    if (!Database::banPoster(application.request(), sourceBoard, postNumber, bans, &err)) {
        Controller::renderErrorNonAjax(application, tq.translate("ActionRoute", "Failed to ban user", "error"), err);
        Tools::log(application, "action/" + action, "fail:" + err, logTarget);
        return;
    }
    redirect();
    Tools::log(application, "action/" + action, "success", logTarget);
}

void ActionRoute::handleBanUser(const QString &action, const Tools::PostParameters &params, const Translator::Qt &tq)
{
    QString ip = params.value("userIp");
    QString logTarget = ip;
    QList<Database::BanInfo> bans;
    foreach (const QString &key, params.keys()) {
        if (!key.startsWith("ban_board_", Qt::CaseInsensitive))
            continue;
        Database::BanInfo inf;
        inf.boardName = params.value(key);
        inf.expires = QDateTime::fromString(params.value("ban_expires" + inf.boardName), "dd.MM.yyyy:hh");
        inf.level = params.value("ban_level_" + inf.boardName).toInt();
        inf.reason = params.value("ban_reason_" + inf.boardName);
        bans << inf;
    }
    QString err;
    if (!Database::banUser(application.request(), ip, bans, &err)) {
        Controller::renderErrorNonAjax(application, tq.translate("ActionRoute", "Failed to ban user", "error"), err);
        Tools::log(application, "action/" + action, "fail:" + err, logTarget);
        return;
    }
    redirect();
    Tools::log(application, "action/" + action, "success", logTarget);
}

void ActionRoute::handleChangeLocale(const QString &action, const Tools::PostParameters &params, const Translator::Qt &)
{
    setCookie("locale", "localeChangeSelect", params);
    redirect();
    Tools::log(application, "action/" + action, "success");
}

void ActionRoute::handleChangeSettings(const QString &action, const Tools::PostParameters &params,
                                       const Translator::Qt &)
{
    setCookie("mode", "modeChangeSelect", params);
    setCookie("style", "styleChangeSelect", params);
    setCookie("time", "timeChangeSelect", params);
    setCookie("captchaEngine", "captchaEngineSelect", params);
    setCookie("maxAllowedRating", "ratingSelect", params);
    setCookie("draftsByDefault", "draftsByDefault", params);
    setCookie("hidePostformRules", "hidePostformRules", params);
    setCookie("timeZoneOffset", "timeZoneOffset", params);
    setCookie("minimalisticPostform", "minimalisticPostform", params);
    QStringList hiddenBoards;
    foreach (const QString &key, params.keys()) {
        if (!key.startsWith("board_") || params.value(key).compare("true", Qt::CaseInsensitive))
            continue;
        hiddenBoards += key.mid(6);
    }
    application.response().set_cookie(cppcms::http::cookie("hiddenBoards", Tools::toStd(hiddenBoards.join("|")),
                                                           UINT_MAX, "/"));
    redirect("settings");
    Tools::log(application, "action/" + action, "success");
}

void ActionRoute::handleCreatePost(const QString &action, const Tools::PostParameters &params,
                                   const Translator::Qt &tq)
{
    AbstractBoard::LockingWrapper board = AbstractBoard::board(params.value("board"));
    if (!testBoard(board.data(), action, params.value("board"), tq))
        return;
    board->createPost(application);
    setCookie("markupMode", "markupMode", params);
}

void ActionRoute::handleCreateThread(const QString &action, const Tools::PostParameters &params,
                                     const Translator::Qt &tq)
{
    AbstractBoard::LockingWrapper board = AbstractBoard::board(params.value("board"));
    if (!testBoard(board.data(), action, params.value("board"), tq))
        return;
    board->createThread(application);
    setCookie("markupMode", "markupMode", params);
}

void ActionRoute::handleDelall(const QString &action, const Tools::PostParameters &params, const Translator::Qt &tq)
{
    QString ip = params.value("userIp");
    QString board = params.value("board");
    QString logTarget = ip + "/" + board;
    Tools::log(application, "action/" + action, "begin", logTarget);
    if (!Controller::testBanNonAjax(application, Controller::WriteAction, board))
        return Tools::log(application, "action/" + action, "fail:ban", logTarget);
    QString err;
    if (!Database::delall(application.request(), ip, board, &err)) {
        Controller::renderErrorNonAjax(application, tq.translate("ActionRoute", "Failed to perform delall",
                                                                 "error"), err);
        Tools::log(application, "action/" + action, "fail:" + err, logTarget);
        return;
    }
    redirect();
    Tools::log(application, "action/" + action, "success", logTarget);
}

void ActionRoute::handleDeleteFile(const QString &action, const Tools::PostParameters &params,
                                   const Translator::Qt &tq)
{
    QString boardName = params.value("boardName");
    quint64 postNumber = params.value("postNumber").toULongLong();
    QString fileName = params.value("fileName");
    QString logTarget = boardName + "/" + QString::number(postNumber) + "/" + fileName;
    if (!Controller::testBanNonAjax(application, Controller::WriteAction, boardName))
        return Tools::log(application, "action/" + action, "fail:ban", logTarget);
    QString err;
    if (!Database::deleteFile(boardName, fileName, application.request(), QByteArray(), &err)) {
        Controller::renderErrorNonAjax(application, tq.translate("ActionRoute", "Failed to delete file", "error"),
                                       err);
        Tools::log(application, "action/" + action, "fail:" + err, logTarget);
        return;
    }
    redirect(boardName + "/thread/" + QString::number(Database::postThreadNumber(boardName, postNumber)) + ".html#"
             + QString::number(postNumber));
    Tools::log(application, "action/" + action, "success", logTarget);
}

void ActionRoute::handleDeletePost(const QString &action, const Tools::PostParameters &params,
                                   const Translator::Qt &tq)
{
    QString boardName = params.value("boardName");
    quint64 postNumber = params.value("postNumber").toULongLong();
    QString logTarget = boardName + "/" + QString::number(postNumber);
    if (!Controller::testBanNonAjax(application, Controller::WriteAction, boardName))
        return Tools::log(application, "action/" + action, "fail:ban", logTarget);
    quint64 threadNumber = Database::postThreadNumber(boardName, postNumber);
    QString err;
    if (!Database::deletePost(boardName, postNumber, application.request(), QByteArray(), &err)) {
        Controller::renderErrorNonAjax(application, tq.translate("ActionRoute", "Failed to delete post", "error"),
                                       err);
        Tools::log(application, "action/" + action, "fail:" + err, logTarget);
        return;
    }
    QString path = boardName;
    if (threadNumber != postNumber)
        path += "/thread/" + QString::number(threadNumber) + ".html#" + QString::number(postNumber);
    redirect(path);
    Tools::log(application, "action/" + action, "success", logTarget);
}

void ActionRoute::handleEditAudioTags(const QString &action, const Tools::PostParameters &params,
                                      const Translator::Qt &tq)
{
    QString boardName = params.value("board");
    quint64 postNumber = params.value("postNumber").toULongLong();
    QString fileName = params.value("fileName");
    QString logTarget = boardName + "/" + QString::number(postNumber) + "/" + fileName;
    if (!Controller::testBanNonAjax(application, Controller::WriteAction, boardName))
        return Tools::log(application, "action/" + action, "fail:ban", logTarget);
    QVariantMap m;
    foreach (const QString &key, QStringList() << "album" << "artist" << "title" << "year")
        m.insert(key, params.value(key));
    QString err;
    if (!Database::editAudioTags(boardName, fileName, application.request(), QByteArray(), m, &err)) {
        Controller::renderErrorNonAjax(application, tq.translate("ActionRoute", "Failed to edit audio tags", "error"),
                                       err);
        Tools::log(application, "action/" + action, "fail:" + err, logTarget);
        return;
    }
    QString path = boardName + "/thread/" + QString::number(Database::postThreadNumber(boardName, postNumber))
            + ".html#" + QString::number(postNumber);
    redirect(path);
    Tools::log(application, "action/" + action, "success", logTarget);
}

void ActionRoute::handleEditPost(const QString &action, const Tools::PostParameters &params, const Translator::Qt &tq)
{
    QString boardName = params.value("board");
    quint64 postNumber = params.value("postNumber").toULongLong();
    Database::EditPostParameters p(application.request(), boardName, postNumber);
    QString logTarget = boardName + "/" + QString::number(p.postNumber);
    if (!Controller::testBanNonAjax(application, Controller::WriteAction, boardName))
        return Tools::log(application, "action/" + action, "fail:ban", logTarget);
    AbstractBoard::LockingWrapper board = AbstractBoard::board(params.value("board"));
    if (!testBoard(board.data(), action, params.value("board"), tq))
        return;
    p.email = params.value("email");
    p.name = params.value("name");
    p.raw = !params.value("raw").compare("true", Qt::CaseInsensitive);
    p.subject = params.value("subject");
    p.text = params.value("text");
    p.draft = !params.value("draft").compare("true", Qt::CaseInsensitive);
    QString mm = params.value("markupMode");
    p.extendedWakabaMarkEnabled = mm.contains("ewm", Qt::CaseInsensitive);
    p.bbCodeEnabled = mm.contains("bbc", Qt::CaseInsensitive);
    p.userData = board->editedPostUserData(params);
    QString err;
    p.error = &err;
    if (!Database::editPost(p)) {
        Controller::renderErrorNonAjax(application, tq.translate("ActionRoute", "Failed to edit post", "error"), err);
        Tools::log(application, "action/" + action, "fail:" + err, logTarget);
        return;
    }
    setCookie("markupMode", "markupMode", params);
    QString path = boardName + "/thread/" + QString::number(Database::postThreadNumber(boardName, postNumber))
            + ".html#" + QString::number(postNumber);
    redirect(path);
    Tools::log(application, "action/" + action, "success", logTarget);
}

void ActionRoute::handleLogin(const QString &action, const Tools::PostParameters &params, const Translator::Qt &)
{
    QString hashpass = params.value("hashpass");
    if (!QRegExp("").exactMatch(hashpass))
        hashpass = Tools::toString(QCryptographicHash::hash(hashpass.toUtf8(), QCryptographicHash::Sha1));
    application.response().set_cookie(cppcms::http::cookie("hashpass", Tools::toStd(hashpass), UINT_MAX, "/"));
    redirect();
    Tools::log(application, "action/" + action, "success");
}

void ActionRoute::handleLogout(const QString &action, const Tools::PostParameters &, const Translator::Qt &)
{
    application.response().set_cookie(cppcms::http::cookie("hashpass", "", UINT_MAX, "/"));
    redirect();
    Tools::log(application, "action/" + action, "success");
}

void ActionRoute::handleMoveThread(const QString &action, const Tools::PostParameters &params,
                                   const Translator::Qt &tq)
{
    QString sourceBoard = params.value("sourceBoard");
    quint64 threadNumber = params.value("threadNumber").toULongLong();
    QString targetBoard = params.value("targetBoard");
    QString logTarget = sourceBoard + "/" + QString::number(threadNumber) + "/" + targetBoard;
    if (!Controller::testBanNonAjax(application, Controller::WriteAction, sourceBoard)
            || !Controller::testBanNonAjax(application, Controller::WriteAction, targetBoard)) {
        return Tools::log(application, "action/" + action, "fail:ban", logTarget);
    }
    QString err;
    quint64 ntn = Database::moveThread(application.request(), sourceBoard, threadNumber, targetBoard, &err);
    if (!ntn) {
        Controller::renderErrorNonAjax(application, tq.translate("ActionRoute", "Failed to move thread",
                                                                 "error"), err);
        Tools::log(application, "action/" + action, "fail:" + err, logTarget);
        return;
    }
    redirect(targetBoard + "/thread/" + QString::number(ntn) + ".html");
    Tools::log(application, "action/" + action, "success", logTarget);
}

void ActionRoute::handleSetThreadFixed(const QString &action, const Tools::PostParameters &params,
                                       const Translator::Qt &tq)
{
    QString boardName = params.value("boardName");
    quint64 threadNumber = params.value("threadNumber").toULongLong();
    bool fixed = !params.value("fixed").compare("true", Qt::CaseSensitive);
    QString logTarget = boardName + "/" + QString::number(threadNumber) + "/" + QString(fixed ? "true" : "false");
    if (!Controller::testBanNonAjax(application, Controller::WriteAction, boardName))
        return Tools::log(application, "action/" + action, "fail:ban", logTarget);
    QString err;
    if (!Database::setThreadFixed(boardName, threadNumber, fixed, application.request(), &err)) {
        Controller::renderErrorNonAjax(application, tq.translate("ActionRoute", "Failed to set thread fixed/unfixed",
                                                                 "error"), err);
        Tools::log(application, "action/" + action, "fail:" + err, logTarget);
        return;
    }
    redirect(boardName + "/thread/" + QString::number(threadNumber) + ".html");
    Tools::log(application, "action/" + action, "success", logTarget);
}

void ActionRoute::handleSetThreadOpened(const QString &action, const Tools::PostParameters &params,
                                        const Translator::Qt &tq)
{
    QString boardName = params.value("boardName");
    quint64 threadNumber = params.value("threadNumber").toULongLong();
    bool opened = !params.value("opened").compare("true", Qt::CaseSensitive);
    QString logTarget = boardName + "/" + QString::number(threadNumber) + "/" + QString(opened ? "true" : "false");
    if (!Controller::testBanNonAjax(application, Controller::WriteAction, boardName))
        return Tools::log(application, "action/" + action, "fail:ban", logTarget);
    QString err;
    if (!Database::setThreadOpened(boardName, threadNumber, opened, application.request(), &err)) {
        Controller::renderErrorNonAjax(application, tq.translate("ActionRoute", "Failed to set thread opened/closed",
                                                                 "error"), err);
        Tools::log(application, "action/" + action, "fail:" + err, logTarget);
        return;
    }
    redirect(boardName + "/thread/" + QString::number(threadNumber) + ".html");
    Tools::log(application, "action/" + action, "success", logTarget);
}

void ActionRoute::handleSetVoteOpened(const QString &action, const Tools::PostParameters &params,
                                      const Translator::Qt &tq)
{
    quint64 postNumber = params.value("postNumber").toULongLong();
    bool opened = !params.value("opened").compare("true", Qt::CaseSensitive);
    QString logTarget = QString::number(postNumber) + "/" + QString(opened ? "true" : "false");
    if (!Controller::testBanNonAjax(application, Controller::WriteAction, "rpg"))
        return Tools::log(application, "action/" + action, "fail:ban", logTarget);
    QString err;
    if (!Database::setVoteOpened(postNumber, opened, QByteArray(), application.request(), &err)) {
        Controller::renderErrorNonAjax(application, tq.translate("ActionRoute", "Failed to set vote opened/closed",
                                                                 "error"), err);
        Tools::log(application, "action/" + action, "fail:" + err, logTarget);
        return;
    }
    redirect("rpg/thread/" + QString::number(Database::postThreadNumber("rpg", postNumber)) + ".html#"
             + QString::number(postNumber));
    Tools::log(application, "action/" + action, "success", logTarget);
}

void ActionRoute::handleUnvote(const QString &action, const Tools::PostParameters &params, const Translator::Qt &tq)
{
    quint64 postNumber = params.value("postNumber").toULongLong();
    QString logTarget = QString::number(postNumber);
    if (!Controller::testBanNonAjax(application, Controller::WriteAction, "rpg"))
        return Tools::log(application, "action/" + action, "fail:ban", logTarget);
    QString err;
    if (!Database::unvote(postNumber, application.request(), &err)) {
        Controller::renderErrorNonAjax(application, tq.translate("ActionRoute", "Failed to unvote", "error"), err);
        Tools::log(application, "action/" + action, "fail:" + err, logTarget);
        return;
    }
    redirect("rpg/thread/" + QString::number(Database::postThreadNumber("rpg", postNumber)) + ".html#"
             + QString::number(postNumber));
    Tools::log(application, "action/" + action, "success", logTarget);
}

void ActionRoute::handleVote(const QString &action, const Tools::PostParameters &params, const Translator::Qt &tq)
{
    quint64 postNumber = params.value("postNumber").toULongLong();
    QString logTarget = QString::number(postNumber);
    if (!Controller::testBanNonAjax(application, Controller::WriteAction, "rpg"))
        return Tools::log(application, "action/" + action, "fail:ban", logTarget);
    QStringList votes;
    if (params.contains("voteGroup")) {
        votes << params.value("voteGroup");
    } else {
        foreach (const QString &key, params.keys()) {
            if (!key.startsWith("voteVariant"))
                continue;
            votes << key.mid(11);
        }
    }
    QString err;
    if (!Database::vote(postNumber, votes, application.request(), &err)) {
        Controller::renderErrorNonAjax(application, tq.translate("ActionRoute", "Failed to vote", "error"), err);
        Tools::log(application, "action/" + action, "fail:" + err, logTarget);
        return;
    }
    redirect("rpg/thread/" + QString::number(Database::postThreadNumber("rpg", postNumber)) + ".html#"
             + QString::number(postNumber));
    Tools::log(application, "action/" + action, "success", logTarget);
}

void ActionRoute::redirect(const QString &path)
{
    Tools::redirect(application, path);
}

void ActionRoute::setCookie(const QString &name, const QString &sourceName, const Tools::PostParameters &params)
{
    if (name.isEmpty() || sourceName.isEmpty())
        return;
    QString value = params.value(sourceName);
    application.response().set_cookie(cppcms::http::cookie(Tools::toStd(name), Tools::toStd(value), UINT_MAX, "/"));
}

bool ActionRoute::testBoard(AbstractBoard *board, const QString &action, const QString &logTarget,
                            const Translator::Qt &tq)
{
    if (board)
        return true;
    QString err = tq.translate("ActionRoute", "Unknown board", "error");
    Controller::renderError(application, err, tq.translate("ActionRoute", "There is no such board", "description"));
    Tools::log(application, "action/" + action, "fail:" + err, logTarget);
    return false;
}
