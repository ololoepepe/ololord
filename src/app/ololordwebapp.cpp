#include "ololordwebapp.h"

#include "route/actionroute.h"
#include "route/boardroute.h"
#include "route/homeroute.h"
#include "route/staticfilesroute.h"
#include "route/threadroute.h"

#include <board/abstractboard.h>
#include <controller/controller.h>
#include <tools.h>

#include <BCoreApplication>
#include <BDirTools>

#include <QDebug>
#include <QString>
#include <QStringList>

#include <cppcms/application.h>
#include <cppcms/service.h>
#include <cppcms/url_dispatcher.h>
#include <cppcms/url_mapper.h>

OlolordWebApp::OlolordWebApp(cppcms::service &service) :
    cppcms::application(service), dynamicFilesRoute("storage/img"), staticFilesRoute("static")
{
    QString boardRx = "(" + AbstractBoard::boardNames().join("|") + ")";
    QString actionRx = "(" + ActionRoute::availableActions().join("|") + ")";
    //
    dispatcher().assign(Tools::toStd("/action/" + actionRx), &OlolordWebApp::action, this, 1);
    mapper().assign("action", "/action/{1}");
    //
    dispatcher().assign(Tools::toStd("/" + boardRx + "/thread/([1-9][0-9]*)\\.html"), &OlolordWebApp::thread, this,
                        1, 2);
    mapper().assign("thread", "/{1}/{2}");
    //
    dispatcher().assign(Tools::toStd("/" + boardRx + "/rules\\.html"), &OlolordWebApp::boardRules, this, 1);
    mapper().assign("boardRules", "/{1}");
    //
    dispatcher().assign(Tools::toStd("/" + boardRx + "/([1-9][0-9]*)\\.html"), &OlolordWebApp::boardPage, this, 1, 2);
    mapper().assign("boardPage", "/{1}/{2}");
    //
    dispatcher().assign(Tools::toStd("/" + boardRx), &OlolordWebApp::board, this, 1);
    mapper().assign("board", "/{1}");
    dispatcher().assign(Tools::toStd("/" + boardRx + "/"), &OlolordWebApp::board, this, 1);
    mapper().assign("board", "/{1}/");
    //
    dispatcher().assign(Tools::toStd("/" + boardRx + "/(.+)"), &OlolordWebApp::dynamicFiles, this, 1, 2);
    mapper().assign("dynamicFiles", "/{1}/{2}");
    //
    dispatcher().assign("/(.+)", &OlolordWebApp::staticFile, this, 1);
    mapper().assign("staticFiles", "/{1}");
    //
    dispatcher().assign("/", &OlolordWebApp::home, this);
    mapper().assign("");
    //
    mapper().root("/board");
}

void OlolordWebApp::main(std::string url)
{
    if (dispatcher().dispatch(url))
        return;
    Tools::log(*this, "Page not found (handled by OlolordWebApp::main)");
    Controller::renderNotFound(*this);
}

void OlolordWebApp::action(std::string path)
{
    actionRoute.handle(*this, Tools::fromStd(path));
}

void OlolordWebApp::board(std::string path)
{
    boardRoute.handle(*this, Tools::fromStd(path));
}

void OlolordWebApp::boardRules(std::string path)
{
    boardRoute.handleRules(*this, Tools::fromStd(path));
}

void OlolordWebApp::boardPage(std::string path1, std::string path2)
{
    boardRoute.handle(*this, Tools::fromStd(path1), Tools::fromStd(path2));
}

void OlolordWebApp::dynamicFiles(std::string path1, std::string path2)
{
    dynamicFilesRoute.handle(*this, Tools::fromStd(path1 + "/" + path2));
}

void OlolordWebApp::home()
{
    homeRoute.handle(*this);
}

void OlolordWebApp::staticFile(std::string path)
{
    staticFilesRoute.handle(*this, Tools::fromStd(path));
}

void OlolordWebApp::thread(std::string path1, std::string path2)
{
    threadRoute.handle(*this, Tools::fromStd(path1), Tools::fromStd(path2));
}
