#ifndef STATICFILESROUTE_H
#define STATICFILESROUTE_H

namespace cppcms
{

class application;

}

#include "abstractroute.h"

#include <QString>

#include <string>

class OLOLORD_EXPORT StaticFilesRoute : public AbstractRoute
{
public:
    enum Mode
    {
        StaticFilesMode = 1,
        DynamicFilesMode
    };
private:
    const Mode mode;
    const QString Prefix;
public:
    explicit StaticFilesRoute(cppcms::application &app, Mode m);
public:
    void handle(std::string path);
    void handle(std::string boardName, std::string path);
    unsigned int handlerArgumentCount() const;
    std::string key() const;
    int priority() const;
    std::string regex() const;
    std::string url() const;
};

#endif // STATICFILESROUTE_H
