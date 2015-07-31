#ifndef RSSROUTE_H
#define RSSROUTE_H

namespace cppcms
{

class application;

}

#include "abstractroute.h"

#include <string>

class OLOLORD_EXPORT RssRoute : public AbstractRoute
{
public:
    explicit RssRoute(cppcms::application &app);
public:
    void handle(std::string boardName);
    unsigned int handlerArgumentCount() const;
    std::string key() const;
    int priority() const;
    std::string regex() const;
    std::string url() const;
};

#endif // RSSROUTE_H
