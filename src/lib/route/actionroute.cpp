#include "actionroute.h"

#include "board/abstractboard.h"
#include "controller/controller.h"
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
    QString a = Tools::fromStd(action);
    Tools::PostParameters params = Tools::postParameters(application.request());
    QString logTarget = params.value("board");
    Tools::log(application, "action/" + a, "begin", logTarget);
    QString err;
    if (!Controller::testRequest(application, Controller::PostRequest, &err))
        return Tools::log(application, "action/" + a, "fail:" + err, logTarget);
    TranslatorQt tq(application.request());
    HandleActionMap map = actionMap();
    if (!map.contains(a)) {
        err = tq.translate("ActionRoute", "Unknown action", "error");
        Controller::renderError(application, err,
                                tq.translate("ActionRoute", "There is no such action", "description"));
        Tools::log(application, "action/" + a, "fail:" + err, logTarget);
        return;
    }
    (this->*map.value(a))(a, params, tq);
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
        map.insert("ban_user", &ActionRoute::handleBanUser);
        map.insert("change_locale", &ActionRoute::handleChangeLocale);
        map.insert("change_settings", &ActionRoute::handleChangeSettings);
        map.insert("create_post", &ActionRoute::handleCreatePost);
        map.insert("create_thread", &ActionRoute::handleCreateThread);
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

void ActionRoute::handleBanUser(const QString &action, const Tools::PostParameters &params, const Translator::Qt &tq)
{
    QString sourceBoard = params.value("boardName");
    quint64 postNumber = params.value("postNumber").toULongLong();
    QString board = params.value("board");
    QString logTarget = sourceBoard + "/" + QString::number(postNumber);
    if (!Controller::testBanNonAjax(application, Controller::WriteAction, sourceBoard)
            || !Controller::testBanNonAjax(application, Controller::WriteAction, board)) {
        return Tools::log(application, "action/" + action, "fail:ban", logTarget);
    }
    QString reason = params.value("reason");
    int level = params.value("level").toInt();
    QDateTime expires = QDateTime::fromString(params.value("expires"), "dd.MM.yyyy:hh");
    QString err;
    if (!Database::banUser(application.request(), sourceBoard, postNumber, board, level, reason, expires, &err)) {
        Controller::renderErrorNonAjax(application, tq.translate("ActionRoute", "Failed to ban user", "error"), err);
        Tools::log(application, "action/" + action, "fail:" + err, logTarget);
        return;
    }
    redirect(sourceBoard + "/thread/" + QString::number(Database::postThreadNumber(sourceBoard, postNumber)) + ".html#"
             + QString::number(postNumber));
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
}

void ActionRoute::handleCreateThread(const QString &action, const Tools::PostParameters &params,
                                     const Translator::Qt &tq)
{
    AbstractBoard::LockingWrapper board = AbstractBoard::board(params.value("board"));
    if (!testBoard(board.data(), action, params.value("board"), tq))
        return;
    board->createThread(application);
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
    p.userData = board->editedPostUserData(params);
    QString err;
    p.error = &err;
    if (!Database::editPost(p)) {
        Controller::renderErrorNonAjax(application, tq.translate("ActionRoute", "Failed to edit post", "error"), err);
        Tools::log(application, "action/" + action, "fail:" + err, logTarget);
        return;
    }
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
            if (!key.startsWith("voteVariant") || params.value(key).compare("true", Qt::CaseInsensitive))
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
