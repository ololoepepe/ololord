#ifndef CONTENT_HOME_H
#define CONTENT_HOME_H

#include "controller/base.h"

#include "../global.h"

#include <list>
#include <string>

namespace Content
{

struct OLOLORD_EXPORT Home : public Base
{
    std::list<std::string> news;
    std::string newsHeader;
    std::list<std::string> rules;
    std::string rulesHeader;
    std::string welcomeMessage;
};

}

#endif // CONTENT_HOME_H
