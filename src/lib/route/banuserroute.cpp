#include "banuserroute.h"

#include "controller.h"
#include "controller/banuser.h"
#include "database.h"
#include "tools.h"
#include "translator.h"

#include <BeQtGlobal>

#include <QDebug>
#include <QString>
#include <QStringList>

#include <cppcms/application.h>
#include <cppcms/http_request.h>

BanUserRoute::BanUserRoute(cppcms::application &app) :
    AbstractRoute(app)
{
    //
}

void BanUserRoute::handle()
{
    DDOS_A(20)
    Tools::GetParameters params = Tools::getParameters(application.request());
    QString boardName = params.value("board");
    quint64 postNumber = params.value("post").toULongLong();
    QString logTarget = boardName + "/" + QString::number(postNumber);
    Tools::log(application, "ban_user", "begin", logTarget);
    QString err;
    if (!Controller::testRequestNonAjax(application, Controller::GetRequest, &err)) {
        Tools::log(application, "ban_user", "fail:" + err, logTarget);
        DDOS_POST_A
        return;
    }
    TranslatorQt tq(application.request());
    if (!Database::moderOnBoard(application.request(), boardName)) {
        QString err = tq.translate("BanUserRoute", "Access error", "error");
        Controller::renderErrorNonAjax(application, err,
                                       tq.translate("BanUserRoute", "Not enough rights", "description"));
        Tools::log(application, "ban_user", "fail:" + err, logTarget);
        DDOS_POST_A
        return;
    }
    if (boardName.isEmpty()) {
        QString err = tq.translate("BanUserRoute", "Invalid board name", "error");
        Controller::renderErrorNonAjax(application, err,
                                       tq.translate("BanUserRoute", "Board name is empty", "description"));
        Tools::log(application, "ban_user", "fail:" + err, logTarget);
        DDOS_POST_A
        return;
    }
    if (!postNumber) {
        QString err = tq.translate("BanUserRoute", "Invalid post number", "error");
        Controller::renderErrorNonAjax(application, err,
                                       tq.translate("BanUserRoute", "Post number is null", "description"));
        Tools::log(application, "ban_user", "fail:" + err, logTarget);
        DDOS_POST_A
        return;
    }
    if (AbstractBoard::board(boardName).isNull()) {
        QString err = tq.translate("BanUserRoute", "Unknown board", "error");
        Controller::renderErrorNonAjax(application, err,
                                       tq.translate("BanUserRoute", "There is no such board", "description"));
        Tools::log(application, "ban_user", "fail:" + err, logTarget);
        DDOS_POST_A
        return;
    }
    Content::BanUser c;
    TranslatorStd ts(application.request());
    Controller::initBase(c, application.request(), tq.translate("BanUserRoute", "Ban user", "pageTitle"));
    QStringList userBoards = Database::registeredUserBoards(application.request());
    if (userBoards.size() == 1 && userBoards.first() == "*")
        userBoards << AbstractBoard::boardNames();
    foreach (const QString &s, userBoards) {
        AbstractBoard::BoardInfo inf;
        inf.name = Tools::toStd(s);
        AbstractBoard::LockingWrapper b = AbstractBoard::board(s);
        inf.title = Tools::toStd(b ? b->title(tq.locale()) : tq.translate("BanUserRoute", "All boards", "boardName"));
        c.availableBoards.push_back(inf);
    }
    c.banExpiresLabelText = ts.translate("BanUserRoute", "Expires:", "banExpiresLabelText");
    c.banLevelLabelText = ts.translate("BanUserRoute", "Level:", "banLevelLabelText");
    Content::BanLevel bl;
    bl.level = 0;
    bl.description = ts.translate("BanUserRoute", "Not banned", "banLevelDesctiption");
    c.banLevels.push_back(bl);
    bl.level = 1;
    bl.description = ts.translate("BanUserRoute", "Posting prohibited", "banLevelDesctiption");
    c.banLevels.push_back(bl);
    bl.level = 10;
    bl.description = ts.translate("BanUserRoute", "Posting and reading prohibited", "banLevelDesctiption");
    c.banLevels.push_back(bl);
    c.banReasonLabelText = ts.translate("BanUserRoute", "Reason:", "banReasonLabelText");
    c.boardLabelText = ts.translate("BanUserRoute", "Board:", "boardLabelText");
    c.currentBoardName = Tools::toStd(boardName);
    c.delallButtonText = ts.translate("BanUserRoute", "Delete all user posts on selected board", "delallButtonText");
    c.postNumber = postNumber;
    c.userIp = Tools::toStd(Database::posterIp(boardName, postNumber));
    bool ok = false;
    QMap<QString, Database::BanInfo> bans = Database::userBanInfo(boardName, postNumber, &ok, &err, tq.locale());
    if (!ok) {
        Controller::renderErrorNonAjax(application, err);
        Tools::log(application, "ban_user", "fail:" + err, logTarget);
        DDOS_POST_A
        return;
    }
    foreach (const QString &bn, userBoards) {
        if ("*" == bn)
            continue;
        Content::BanUser::BanInfo info;
        AbstractBoard::LockingWrapper b = AbstractBoard::board(bn);
        if (b.isNull()) {
            QString err = tq.translate("BanUserRoute", "Internal error", "error");
            Controller::renderErrorNonAjax(application, err);
            Tools::log(application, "ban_user", "fail:" + err, logTarget);
            DDOS_POST_A
            return;
        }
        info.boardName = Tools::toStd(bn);
        info.boardTitle = Tools::toStd(b->title(tq.locale()));
        if (bans.contains(bn)) {
            Database::BanInfo inf = bans.value(bn);
            info.dateTime = Tools::toStd(Tools::dateTime(inf.dateTime,
                                                         application.request()).toString("dd.MM.yyyy-hh:mm:ss"));
            info.expires = Tools::toStd(Tools::dateTime(inf.expires, application.request()).toString("dd.MM.yyyy:hh"));
            info.level = inf.level;
            info.reason = Tools::toStd(inf.reason);
        } else {
            info.dateTime.clear();
            info.expires.clear();
            info.level = 0;
            info.reason.clear();
        }
        c.bans.push_back(info);
    }
    Tools::render(application, "ban_user", c);
    Tools::log(application, "ban_user", "success", logTarget);
    DDOS_POST_A
}

unsigned int BanUserRoute::handlerArgumentCount() const
{
    return 0;
}

std::string BanUserRoute::key() const
{
    return "ban_user";
}

int BanUserRoute::priority() const
{
    return 0;
}

std::string BanUserRoute::regex() const
{
    return "/ban_user";
}

std::string BanUserRoute::url() const
{
    return "/ban_user";
}
