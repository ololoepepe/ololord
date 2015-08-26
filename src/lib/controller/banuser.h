#ifndef CONTENT_BANUSER_H
#define CONTENT_BANUSER_H

#include "controller/base.h"
#include "controller/baseboard.h"

#include "../global.h"

#include <cppcms/view.h>

#include <list>
#include <map>
#include <string>

namespace Content
{

struct OLOLORD_EXPORT BanUser : public Base
{
    struct BanInfo
    {
        std::string boardName;
        std::string boardTitle;
        std::string dateTime;
        std::string expires;
        int level;
        std::string reason;
    };
public:
    std::list<AbstractBoard::BoardInfo> availableBoards;
    std::string banExpiresLabelText;
    std::string banLevelLabelText;
    std::list<BanLevel> banLevels;
    std::string banReasonLabelText;
    std::list<BanInfo> bans;
    std::string boardLabelText;
    std::string currentBoardName;
    std::string delallButtonText;
    unsigned long long postNumber;
    std::string userIp;
};

}

#endif // CONTENT_BANUSER_H
