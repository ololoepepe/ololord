#ifndef SEARCHROUTE_H
#define SEARCHROUTE_H

namespace cppcms
{

class application;

}

#include "abstractroute.h"

#include <string>

class OLOLORD_EXPORT SearchRoute : public AbstractRoute
{
public:
    explicit SearchRoute(cppcms::application &app);
public:
    void handle();
    unsigned int handlerArgumentCount() const;
    std::string key() const;
    int priority() const;
    std::string regex() const;
    std::string url() const;
};

#endif // SEARCHROUTE_H
