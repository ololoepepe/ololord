#include "manageroute.h"

#include "controller.h"
#include "controller/manage.h"
#include "database.h"
#include "tools.h"
#include "translator.h"

#include <BeQtGlobal>

#include <QDebug>
#include <QString>
#include <QStringList>

#include <cppcms/application.h>
#include <cppcms/http_request.h>

ManageRoute::ManageRoute(cppcms::application &app) :
    AbstractRoute(app)
{
    //
}

void ManageRoute::handle()
{
    DDOS_A(16)
    Tools::log(application, "manage", "begin");
    QString err;
    if (!Controller::testRequestNonAjax(application, Controller::GetRequest, &err)) {
        Tools::log(application, "manage", "fail:" + err);
        DDOS_POST_A
        return;
    }
    TranslatorQt tq(application.request());
    if (Database::registeredUserLevel(application.request()) < RegisteredUser::ModerLevel) {
        QString err = tq.translate("ManageRoute", "Access error", "error");
        Controller::renderErrorNonAjax(application, err,
                                       tq.translate("ManageRoute", "Not enough rights", "description"));
        Tools::log(application, "manage", "fail:" + err);
        DDOS_POST_A
        return;
    }
    Content::Manage c;
    TranslatorStd ts(application.request());
    Controller::initBase(c, application.request(), tq.translate("ManageRoute", "User management", "pageTitle"));
    QStringList userBoards = Database::registeredUserBoards(application.request());
    if (userBoards.size() == 1 && userBoards.first() == "*")
        userBoards << AbstractBoard::boardNames();
    foreach (const QString &s, userBoards) {
        AbstractBoard::BoardInfo inf;
        inf.name = Tools::toStd(s);
        AbstractBoard::LockingWrapper b = AbstractBoard::board(s);
        inf.title = Tools::toStd(b ? b->title(tq.locale()) : tq.translate("ManageRoute", "All boards", "boardName"));
        c.availableBoards.push_back(inf);
    }
    c.banExpiresLabelText = ts.translate("ManageRoute", "Expires:", "banExpiresLabelText");
    c.banLevelLabelText = ts.translate("ManageRoute", "Level:", "banLevelLabelText");
    Content::BanLevel bl;
    bl.level = 0;
    bl.description = ts.translate("ManageRoute", "Not banned", "banLevelDesctiption");
    c.banLevels.push_back(bl);
    bl.level = 1;
    bl.description = ts.translate("ManageRoute", "Posting prohibited", "banLevelDesctiption");
    c.banLevels.push_back(bl);
    bl.level = 10;
    bl.description = ts.translate("ManageRoute", "Posting and reading prohibited", "banLevelDesctiption");
    c.banLevels.push_back(bl);
    c.banReasonLabelText = ts.translate("ManageRoute", "Reason:", "banReasonLabelText");
    c.boardLabelText = ts.translate("ManageRoute", "Board:", "boardLabelText");
    c.delallButtonText = ts.translate("ManageRoute", "Delete all user posts on selected board", "delallButtonText");
    c.selectAllText = ts.translate("ManageRoute", "Select all", "selectAllText");
    bool ok = false;
    QMap< QString, QMap<QString, Database::BanInfo> > banInfos = Database::banInfos(&ok, &err, tq.locale());
    if (!ok) {
        Controller::renderErrorNonAjax(application, err);
        Tools::log(application, "manage", "fail:" + err);
        DDOS_POST_A
        return;
    }
    foreach (const QString &ip, banInfos.keys()) {
        Content::Manage::BannedUser user;
        user.ip = Tools::toStd(ip);
        foreach (const QString &bn, userBoards) {
            if ("*" == bn)
                continue;
            Content::Manage::BanInfo info;
            AbstractBoard::LockingWrapper b = AbstractBoard::board(bn);
            if (b.isNull()) {
                QString err = tq.translate("ManageRoute", "Internal error", "error");
                Controller::renderErrorNonAjax(application, err);
                Tools::log(application, "manage", "fail:" + err);
                DDOS_POST_A
                return;
            }
            info.boardName = Tools::toStd(bn);
            info.boardTitle = Tools::toStd(b->title(tq.locale()));
            QMap<QString, Database::BanInfo> bans = banInfos.value(ip);
            if (bans.contains(bn)) {
                Database::BanInfo inf = bans.value(bn);
                info.dateTime = Tools::toStd(Tools::dateTime(inf.dateTime,
                                                             application.request()).toString("dd.MM.yyyy-hh:mm:ss"));
                info.expires = Tools::toStd(Tools::dateTime(inf.expires,
                                                            application.request()).toString("dd.MM.yyyy:hh"));
                info.level = inf.level;
                info.reason = Tools::toStd(inf.reason);
            } else {
                info.dateTime.clear();
                info.expires.clear();
                info.level = 0;
                info.reason.clear();
            }
            user.bans.push_back(info);
        }
        c.bannedUsers.push_back(user);
    }
    Tools::render(application, "manage", c);
    Tools::log(application, "manage", "success");
    DDOS_POST_A
}

unsigned int ManageRoute::handlerArgumentCount() const
{
    return 0;
}

std::string ManageRoute::key() const
{
    return "manage";
}

int ManageRoute::priority() const
{
    return 0;
}

std::string ManageRoute::regex() const
{
    return "/manage";
}

std::string ManageRoute::url() const
{
    return "/manage";
}
