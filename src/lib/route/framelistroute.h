#ifndef FRAMELISTROUTE_H
#define FRAMELISTROUTE_H

namespace cppcms
{

class application;

}

#include "abstractroute.h"

#include <string>

class OLOLORD_EXPORT FrameListRoute : public AbstractRoute
{
public:
    explicit FrameListRoute(cppcms::application &app);
public:
    void handle();
    unsigned int handlerArgumentCount() const;
    std::string key() const;
    int priority() const;
    std::string regex() const;
    std::string url() const;
};

#endif // FRAMELISTROUTE_H
