#ifndef OLOLORDWEBAPP_H
#define OLOLORDWEBAPP_H

#include "route/actionroute.h"
#include "route/boardroute.h"
#include "route/homeroute.h"
#include "route/staticfilesroute.h"
#include "route/threadroute.h"

namespace cppcms
{

class service;

}

#include <cppcms/application.h>

#include <string>

class OlolordWebApp : public cppcms::application
{
private:
    ActionRoute actionRoute;
    BoardRoute boardRoute;
    StaticFilesRoute dynamicFilesRoute;
    HomeRoute homeRoute;
    StaticFilesRoute staticFilesRoute;
    ThreadRoute threadRoute;
public:
    explicit OlolordWebApp(cppcms::service &service);
public:
    void main(std::string url);
protected:
    void action(std::string path);
    void board(std::string path);
    void boardPage(std::string path1, std::string path2);
    void boardRules(std::string path);
    void dynamicFiles(std::string path1, std::string path2);
    void home();
    void staticFile(std::string path);
    void thread(std::string path1, std::string path2);
};

#endif // OLOLORDWEBAPP_H
