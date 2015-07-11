#ifndef CONTENT_BANUSER_H
#define CONTENT_BANUSER_H

#include "controller/base.h"
#include "controller/baseboard.h"

#include "../global.h"

#include <cppcms/view.h>

#include <list>
#include <string>

namespace Content
{

struct OLOLORD_EXPORT BanUser : public Base
{
    std::list<AbstractBoard::BoardInfo> availableBoards;
    std::string banExpiresLabelText;
    std::string banLevelLabelText;
    std::list<BanLevel> banLevels;
    std::string banReasonLabelText;
    std::string boardLabelText;
    std::string currentBoardName;
    unsigned long long postNumber;
};

}

#endif // CONTENT_BANUSER_H
