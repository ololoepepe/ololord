#ifndef CONTENT_MANAGE_H
#define CONTENT_MANAGE_H

#include "controller/base.h"
#include "controller/baseboard.h"

#include "../global.h"

#include <cppcms/view.h>

#include <list>
#include <string>

namespace Content
{

struct OLOLORD_EXPORT Manage : public Base
{
    std::list<AbstractBoard::BoardInfo> availableBoards;
    std::string banExpiresLabelText;
    std::string banLevelLabelText;
    std::list<BanLevel> banLevels;
    std::string banReasonLabelText;
    std::string boardLabelText;
};

}

#endif // CONTENT_MANAGE_H
