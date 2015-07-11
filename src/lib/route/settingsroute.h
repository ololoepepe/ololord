#ifndef SETTINGSROUTE_H
#define SETTINGSROUTE_H

namespace cppcms
{

class application;

}

#include "abstractroute.h"

#include <string>

class OLOLORD_EXPORT SettingsRoute : public AbstractRoute
{
public:
    explicit SettingsRoute(cppcms::application &app);
public:
    void handle();
    unsigned int handlerArgumentCount() const;
    std::string key() const;
    int priority() const;
    std::string regex() const;
    std::string url() const;
};

#endif // SETTINGSROUTE_H
