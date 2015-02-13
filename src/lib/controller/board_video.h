#ifndef CONTENT_BOARD_VIDEO_H
#define CONTENT_BOARD_VIDEO_H

#include "controller/withbase.h"
#include "../global.h"

#include <cppcms/view.h>

#include <string>

namespace Content
{

struct OLOLORD_EXPORT BoardVideo : public cppcms::base_content, public WithBase
{
    std::string altVideoText;
    std::string pageTitle;
    std::string videoFileName;
    std::string videoType;
};

}

#endif // CONTENT_BOARD_VIDEO_H
