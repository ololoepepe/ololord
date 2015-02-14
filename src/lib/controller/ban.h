#ifndef CONTENT_BAN_H
#define CONTENT_BAN_H

#include "controller/base.h"

#include "../global.h"

#include <cppcms/view.h>

#include <string>

namespace Content
{

struct OLOLORD_EXPORT Ban : public Base
{
    std::string banBoard;
    std::string banBoardLabel;
    std::string banDateTime;
    std::string banDateTimeLabel;
    std::string banExpires;
    std::string banExpiresLabel;
    std::string banLevel;
    std::string banLevelLabel;
    std::string banMessage;
    std::string banReason;
    std::string banReasonLabel;
};

}

#endif // CONTENT_BAN_H
