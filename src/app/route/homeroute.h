#ifndef HOMEROUTE_H
#define HOMEROUTE_H

#include <QCoreApplication>

namespace cppcms
{

class application;

}

class HomeRoute
{
    Q_DECLARE_TR_FUNCTIONS(HomeRoute)
public:
    explicit HomeRoute();
public:
    void handle(cppcms::application &app);
};

#endif // HOMEROUTE_H
