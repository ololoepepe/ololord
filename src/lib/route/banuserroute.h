#ifndef BANUSERROUTE_H
#define BANUSERROUTE_H

namespace cppcms
{

class application;

}

#include "abstractroute.h"

#include <string>

class OLOLORD_EXPORT BanUserRoute : public AbstractRoute
{
public:
    explicit BanUserRoute(cppcms::application &app);
public:
    void handle();
    unsigned int handlerArgumentCount() const;
    std::string key() const;
    int priority() const;
    std::string regex() const;
    std::string url() const;
};

#endif // BANUSERROUTE_H
