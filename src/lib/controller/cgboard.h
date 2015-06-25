#ifndef CONTENT_BOARD_IMAGE_H
#define CONTENT_BOARD_IMAGE_H

#include "controller/base.h"

#include "../global.h"

#include <string>

namespace Content
{

struct OLOLORD_EXPORT cgBoard : public Base
{
    std::string imageFileName;
    std::string imageTitle;
    std::string noJokeButtonText;
};

}

#endif // CONTENT_BOARD_IMAGE_H
