#ifndef WITHNAVBAR_H
#define WITHNAVBAR_H

#include "board/abstractboard.h"
#include "../global.h"

#include <string>

struct OLOLORD_EXPORT WithNavbar
{
    AbstractBoard::BoardInfoList boards;
    std::string toHomePageText;
};

#endif // WITHNAVBAR_H
