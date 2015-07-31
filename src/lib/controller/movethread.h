#ifndef CONTENT_MOVETHREAD_H
#define CONTENT_MOVETHREAD_H

#include "controller/base.h"
#include "controller/baseboard.h"

#include "../global.h"

#include <cppcms/view.h>

#include <string>

namespace Content
{

struct OLOLORD_EXPORT MoveThread : public Base
{
    std::list<AbstractBoard::BoardInfo> availableBoards;
    std::string currentBoardName;
    std::string moveThreadWarningText;
    unsigned long long threadNumber;
};

}

#endif // CONTENT_MOVETHREAD_H
