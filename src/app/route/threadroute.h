#ifndef THREADROUTE_H
#define THREADROUTE_H

#include "board/abstractboard.h"

class QString;

namespace cppcms
{

class application;

}

class ThreadRoute
{
public:
    explicit ThreadRoute();
public:
    void handle(cppcms::application &app, const QString &boardName, const QString &thread);
};

#endif // THREADROUTE_H
