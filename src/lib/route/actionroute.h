#ifndef ACTIONROUTE_H
#define ACTIONROUTE_H

class QStringList;

namespace cppcms
{

class application;

}

#include "abstractroute.h"

#include <string>

class OLOLORD_EXPORT ActionRoute : public AbstractRoute
{
public:
    explicit ActionRoute(cppcms::application &app);
public:
    static QStringList availableActions();
public:
    void handle(std::string action);
    unsigned int handlerArgumentCount() const;
    std::string key() const;
    int priority() const;
    std::string regex() const;
    std::string url() const;
};

#endif // ACTIONROUTE_H
