#ifndef MARKUPROUTE_H
#define MARKUPROUTE_H

namespace cppcms
{

class application;

}

#include "abstractroute.h"

#include <string>

class OLOLORD_EXPORT MarkupRoute : public AbstractRoute
{
public:
    explicit MarkupRoute(cppcms::application &app);
public:
    void handle();
    unsigned int handlerArgumentCount() const;
    std::string key() const;
    int priority() const;
    std::string regex() const;
    std::string url() const;
};

#endif // MARKUPROUTE_H
