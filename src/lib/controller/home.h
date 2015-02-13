#ifndef CONTENT_HOME_H
#define CONTENT_HOME_H

#include "controller/withbase.h"
#include "controller/withnavbar.h"
#include "controller/withsettings.h"
#include "../global.h"

#include <cppcms/view.h>

#include <list>
#include <string>

namespace Content
{

struct OLOLORD_EXPORT Home : public cppcms::base_content, public WithBase, public WithNavbar, public WithSettings
{
    std::string pageTitle;
    std::list<std::string> rules;
    std::string welcomeMessage;
};

}

#endif // CONTENT_HOME_H
