#ifndef IPBAN_H
#define IPBAN_H

#include "controller/base.h"

#include "../global.h"

#include <cppcms/view.h>

#include <string>

namespace Content
{

struct OLOLORD_EXPORT IpBan : public Base
{
    std::string banDescription;
    std::string banMessage;
};

}

#endif // IPBAN_H
