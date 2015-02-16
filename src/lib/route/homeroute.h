#ifndef HOMEROUTE_H
#define HOMEROUTE_H

namespace cppcms
{

class application;

}

#include "abstractroute.h"

#include <string>

class OLOLORD_EXPORT HomeRoute : public AbstractRoute
{
public:
    explicit HomeRoute(cppcms::application &app);
public:
    void handle();
    unsigned int handlerArgumentCount() const;
    std::string key() const;
    int priority() const;
    std::string regex() const;
    std::string url() const;
};

#endif // HOMEROUTE_H
