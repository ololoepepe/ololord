#ifndef ADDFILEROUTE_H
#define ADDFILEROUTE_H

namespace cppcms
{

class application;

}

#include "abstractroute.h"

#include <string>

class OLOLORD_EXPORT AddFileRoute : public AbstractRoute
{
public:
    explicit AddFileRoute(cppcms::application &app);
public:
    void handle();
    unsigned int handlerArgumentCount() const;
    std::string key() const;
    int priority() const;
    std::string regex() const;
    std::string url() const;
};

#endif // ADDFILEROUTE_H
