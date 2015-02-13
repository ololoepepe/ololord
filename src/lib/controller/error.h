#ifndef ERROR_H
#define ERROR_H

#include "controller/withbase.h"
#include "controller/withnavbar.h"
#include "controller/withsettings.h"
#include "../global.h"

#include <cppcms/view.h>

#include <list>
#include <string>

namespace Content
{

struct OLOLORD_EXPORT Error : public cppcms::base_content, public WithBase, public WithNavbar, public WithSettings
{
    std::string errorDescription;
    std::string errorMessage;
    std::string pageTitle;
};

}

#endif // ERROR_H
