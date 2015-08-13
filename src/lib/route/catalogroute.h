#ifndef CATALOGROUTE_H
#define CATALOGROUTE_H

namespace cppcms
{

class application;

}

#include "abstractroute.h"

#include <string>

class OLOLORD_EXPORT CatalogRoute : public AbstractRoute
{
public:
    explicit CatalogRoute(cppcms::application &app);
public:
    void handle(std::string boardName);
    unsigned int handlerArgumentCount() const;
    std::string key() const;
    int priority() const;
    std::string regex() const;
    std::string url() const;
};

#endif // CATALOGROUTE_H
