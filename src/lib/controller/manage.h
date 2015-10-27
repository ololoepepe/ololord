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
    struct BanInfo
    {
        std::string boardName;
        std::string boardTitle;
        std::string dateTime;
        std::string expires;
        int level;
        std::string reason;
    };
    struct BannedUser
    {
        std::string ip;
        std::list<BanInfo> bans;
    };
public:
    std::list<AbstractBoard::BoardInfo> availableBoards;
    std::string banExpiresLabelText;
    std::string banLevelLabelText;
    std::list<BanLevel> banLevels;
    std::list<BannedUser> bannedUsers;
    std::string banReasonLabelText;
    std::string boardLabelText;
    std::string delallButtonText;
    std::string selectAllText;
};

}

#endif // CONTENT_MANAGE_H
