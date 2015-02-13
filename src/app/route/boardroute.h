#ifndef BOARDROUTE_H
#define BOARDROUTE_H

#include "board/abstractboard.h"

class QString;

namespace cppcms
{

class application;

}

class BoardRoute
{
public:
    explicit BoardRoute();
public:
    void handle(cppcms::application &app, const QString &boardName);
    void handle(cppcms::application &app, const QString &boardName, const QString &page);
    void handleRules(cppcms::application &app, const QString &boardName);
};

#endif // BOARDROUTE_H
