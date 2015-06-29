#ifndef CONTENT_FRAMELIST_H
#define CONTENT_FRAMELIST_H

#include "controller/base.h"

#include "../global.h"

#include <cppcms/view.h>

#include <string>

namespace Content
{

struct OLOLORD_EXPORT FrameList : public Base
{
    std::string normalVersionText;
};

}

#endif // CONTENT_FRAMELIST_H
