#ifndef STATICFILESROUTE_H
#define STATICFILESROUTE_H

namespace cppcms
{

class application;

}

#include <QString>

class StaticFilesRoute
{
public:
    const QString Prefix;
public:
    explicit StaticFilesRoute(const QString &prefix);
public:
    void handle(cppcms::application &app, const QString &path);
};

#endif // STATICFILESROUTE_H
