#ifndef CONTENT_BOARD_IMAGE_H
#define CONTENT_BOARD_IMAGE_H

#include "controller/withbase.h"
#include "../global.h"

#include <cppcms/view.h>

#include <string>

namespace Content
{

struct OLOLORD_EXPORT BoardImage : public cppcms::base_content, public WithBase
{
    std::string imageFileName;
    std::string imageTitle;
    std::string pageTitle;
};

}

#endif // CONTENT_BOARD_IMAGE_H
