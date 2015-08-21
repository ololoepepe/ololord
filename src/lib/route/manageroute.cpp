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
    Tools::log(application, "manage", "begin");
    QString err;
    if (!Controller::testRequestNonAjax(application, Controller::GetRequest, &err))
        return Tools::log(application, "manage", "fail:" + err);
    TranslatorQt tq(application.request());
    /*if (!Database::moderOnBoard(application.request(), boardName)) {
        QString err = tq.translate("BanUserRoute", "Access error", "error");
        Controller::renderErrorNonAjax(application, err,
                                       tq.translate("BanUserRoute", "Not enough rights", "description"));
        Tools::log(application, "ban_user", "fail:" + err, logTarget);
        return;
    }*/
    Content::Manage c;
    TranslatorStd ts(application.request());
    Controller::initBase(c, application.request(), tq.translate("ManageRoute", "Board management", "pageTitle"));
    QStringList userBoards = Database::registeredUserBoards(application.request());
    if (userBoards.size() == 1 && userBoards.first() == "*")
        userBoards << AbstractBoard::boardNames();
    foreach (const QString &s, userBoards) {
        AbstractBoard::BoardInfo inf;
        inf.name = Tools::toStd(s);
        AbstractBoard::LockingWrapper b = AbstractBoard::board(s);
        inf.title = Tools::toStd(b ? b->title(tq.locale()) : tq.translate("ManagerRoute", "All boards", "boardName"));
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
    Tools::render(application, "manage", c);
    Tools::log(application, "manage", "success");
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
