#ifndef MANAGEROUTE_H
#define MANAGEROUTE_H

namespace cppcms
{

class application;

}

#include "abstractroute.h"

#include <string>

class OLOLORD_EXPORT ManageRoute : public AbstractRoute
{
public:
    explicit ManageRoute(cppcms::application &app);
public:
    void handle();
    unsigned int handlerArgumentCount() const;
    std::string key() const;
    int priority() const;
    std::string regex() const;
    std::string url() const;
};

#endif // MANAGEROUTE_H
