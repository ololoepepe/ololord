#ifndef ACTIONROUTE_H
#define ACTIONROUTE_H

class QString;
class QStringList;

namespace cppcms
{

class application;

}

class ActionRoute
{
public:
    explicit ActionRoute();
public:
    static QStringList availableActions();
public:
    void handle(cppcms::application &app, const QString &action);
};

#endif // ACTIONROUTE_H
