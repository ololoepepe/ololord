#ifndef EDITPOSTROUTE_H
#define EDITPOSTROUTE_H

namespace cppcms
{

class application;

}

#include "abstractroute.h"

#include <string>

class OLOLORD_EXPORT EditPostRoute : public AbstractRoute
{
public:
    explicit EditPostRoute(cppcms::application &app);
public:
    void handle();
    unsigned int handlerArgumentCount() const;
    std::string key() const;
    int priority() const;
    std::string regex() const;
    std::string url() const;
};

#endif // EDITPOSTROUTE_H
