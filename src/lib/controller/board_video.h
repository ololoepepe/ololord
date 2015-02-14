#ifndef CONTENT_BOARD_VIDEO_H
#define CONTENT_BOARD_VIDEO_H

#include "controller/base.h"

#include "../global.h"

#include <string>

namespace Content
{

struct OLOLORD_EXPORT BoardVideo : public Base
{
    std::string altVideoText;
    std::string videoFileName;
    std::string videoType;
};

}

#endif // CONTENT_BOARD_VIDEO_H
