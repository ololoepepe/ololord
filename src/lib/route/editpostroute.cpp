#include "editpostroute.h"

#include "board/abstractboard.h"
#include "controller.h"
#include "tools.h"
#include "translator.h"

#include <BeQtGlobal>

#include <QDebug>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QVariantMap>

#include <cppcms/application.h>
#include <cppcms/http_request.h>

EditPostRoute::EditPostRoute(cppcms::application &app) :
    AbstractRoute(app)
{
    //
}

void EditPostRoute::handle()
{
    DDOS_A(18)
    Tools::GetParameters params = Tools::getParameters(application.request());
    QString boardName = params.value("board");
    quint64 postNumber = params.value("post").toULongLong();
    QString logTarget = boardName + "/" + QString::number(postNumber);
    Tools::log(application, "edit_post", "begin", logTarget);
    QString err;
    TranslatorQt tq(application.request());
    if (!Controller::testRequestNonAjax(application, Controller::GetRequest, &err)) {
        Tools::log(application, "edit_post", "fail:" + err, logTarget);
        DDOS_POST_A
        return;
    }
    if (Tools::hashpassString(application.request()).isEmpty()) {
        QString err = tq.translate("EditPostRoute", "Access error", "error");
        Controller::renderErrorNonAjax(application, err,
                                       tq.translate("EditPostRoute", "Not enough rights", "description"));
        Tools::log(application, "edit_post", "fail:" + err, logTarget);
        DDOS_POST_A
        return;
    }
    if (boardName.isEmpty()) {
        QString err = tq.translate("EditPostRoute", "Invalid board name", "error");
        Controller::renderErrorNonAjax(application, err,
                                       tq.translate("EditPostRoute", "Board name is empty", "description"));
        Tools::log(application, "edit_post", "fail:" + err, logTarget);
        DDOS_POST_A
        return;
    }
    AbstractBoard::LockingWrapper board = AbstractBoard::board(boardName);
    if (board.isNull()) {
        QString err = tq.translate("EditPostRoute", "Unknown board", "error");
        Controller::renderErrorNonAjax(application, err,
                                       tq.translate("EditPostRoute", "There is no such board", "description"));
        Tools::log(application, "edit_post", "fail:" + err, logTarget);
        DDOS_POST_A
        return;
    }
    board->handleEditPost(application, postNumber);
    DDOS_POST_A
}

unsigned int EditPostRoute::handlerArgumentCount() const
{
    return 0;
}

std::string EditPostRoute::key() const
{
    return "edit_post";
}

int EditPostRoute::priority() const
{
    return 0;
}

std::string EditPostRoute::regex() const
{
    return "/edit_post";
}

std::string EditPostRoute::url() const
{
    return "/edit_post";
}
