#ifndef CONTENT_RULES_H
#define CONTENT_RULES_H

#include "controller/base.h"

#include "../global.h"
#include "board/abstractboard.h"

#include <cppcms/view.h>

#include <list>
#include <string>

namespace Content
{

struct OLOLORD_EXPORT Rules : public Base
{
    AbstractBoard::BoardInfo currentBoard;
    std::string noRulesText;
    std::list<std::string> rules;
};

}

#endif // CONTENT_RULES_H
