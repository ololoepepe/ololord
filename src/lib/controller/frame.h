#ifndef CONTENT_FRAME_H
#define CONTENT_FRAME_H

#include "controller/base.h"

#include "../global.h"

#include <cppcms/view.h>

#include <string>

namespace Content
{

struct OLOLORD_EXPORT Frame : public Base
{
    std::string sourcePath;
};

}

#endif // CONTENT_FRAME_H
