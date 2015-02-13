#ifndef CONTENT_BAN_H
#define CONTENT_BAN_H

#include "controller/withbase.h"
#include "controller/withnavbar.h"
#include "controller/withsettings.h"
#include "../global.h"

#include <cppcms/view.h>

#include <string>

namespace Content
{

struct OLOLORD_EXPORT Ban : public cppcms::base_content, public WithBase, public WithNavbar, public WithSettings
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
    std::string pageTitle;
};

}

#endif // CONTENT_BAN_H
