#ifndef CONTENT_NOTFOUND_H
#define CONTENT_NOTFOUND_H

#include "controller/withbase.h"
#include "controller/withnavbar.h"
#include "controller/withsettings.h"
#include "../global.h"

#include <cppcms/view.h>

#include <list>
#include <string>

namespace Content
{

struct OLOLORD_EXPORT NotFound : public cppcms::base_content, public WithBase, public WithNavbar, public WithSettings
{
    std::string imageFileName;
    std::string imageTitle;
    std::string notFoundMessage;
    std::string pageTitle;
};

}

#endif // CONTENT_NOTFOUND_H
