#ifndef PLAYLISTROUTE_H
#define PLAYLISTROUTE_H

namespace cppcms
{

class application;

}

#include "abstractroute.h"

#include <string>

class OLOLORD_EXPORT PlaylistRoute : public AbstractRoute
{
public:
    explicit PlaylistRoute(cppcms::application &app);
public:
    void handle();
    unsigned int handlerArgumentCount() const;
    std::string key() const;
    int priority() const;
    std::string regex() const;
    std::string url() const;
};

#endif // PLAYLISTROUTE_H
