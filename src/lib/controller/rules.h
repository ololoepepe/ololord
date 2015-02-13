#ifndef CONTENT_RULES_H
#define CONTENT_RULES_H

#include "board/abstractboard.h"
#include "controller/withbase.h"
#include "controller/withnavbar.h"
#include "controller/withsettings.h"
#include "../global.h"

#include <cppcms/view.h>

#include <list>
#include <string>

namespace Content
{

struct OLOLORD_EXPORT Rules : public cppcms::base_content, public WithBase, public WithNavbar, public WithSettings
{
    AbstractBoard::BoardInfo currentBoard;
    std::string noRulesText;
    std::string pageTitle;
    std::list<std::string> rules;
};

}

#endif // CONTENT_RULES_H
