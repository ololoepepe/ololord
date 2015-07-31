#ifndef MOVETHREADROUTE_H
#define MOVETHREADROUTE_H

namespace cppcms
{

class application;

}

#include "abstractroute.h"

#include <string>

class OLOLORD_EXPORT MoveThreadRoute : public AbstractRoute
{
public:
    explicit MoveThreadRoute(cppcms::application &app);
public:
    void handle();
    unsigned int handlerArgumentCount() const;
    std::string key() const;
    int priority() const;
    std::string regex() const;
    std::string url() const;
};

#endif // MOVETHREADROUTE_H
