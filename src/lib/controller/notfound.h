#ifndef CONTENT_NOTFOUND_H
#define CONTENT_NOTFOUND_H

#include "controller/base.h"

#include "../global.h"

#include <cppcms/view.h>

#include <list>
#include <string>

namespace Content
{

struct OLOLORD_EXPORT NotFound : public Base
{
    std::string imageFileName;
    std::string imageTitle;
    std::string notFoundMessage;
};

}

#endif // CONTENT_NOTFOUND_H
