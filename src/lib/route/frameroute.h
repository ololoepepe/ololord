#ifndef FRAMEROUTE_H
#define FRAMEROUTE_H

namespace cppcms
{

class application;

}

#include "abstractroute.h"

#include <string>

class OLOLORD_EXPORT FrameRoute : public AbstractRoute
{
public:
    explicit FrameRoute(cppcms::application &app);
public:
    void handle();
    unsigned int handlerArgumentCount() const;
    std::string key() const;
    int priority() const;
    std::string regex() const;
    std::string url() const;
};

#endif // FRAMEROUTE_H
