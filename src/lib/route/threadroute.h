#ifndef THREADROUTE_H
#define THREADROUTE_H

namespace cppcms
{

class application;

}

#include "abstractroute.h"

#include <string>

class OLOLORD_EXPORT ThreadRoute : public AbstractRoute
{
public:
    explicit ThreadRoute(cppcms::application &app);
public:
    void handle(std::string boardName, std::string thread);
    unsigned int handlerArgumentCount() const;
    std::string key() const;
    int priority() const;
    std::string regex() const;
    std::string url() const;
};

#endif // THREADROUTE_H
