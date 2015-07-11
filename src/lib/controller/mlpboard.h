#ifndef CONTENT_MLP_BOARD_H
#define CONTENT_MLP_BOARD_H

#include "controller/base.h"

#include "../global.h"

#include <string>

namespace Content
{

struct OLOLORD_EXPORT mlpBoard : public Base
{
    std::string altVideoText;
    std::string buttonText;
    std::string noJokeButtonText;
    std::string videoFileName;
    std::string videoFileName2;
    std::string videoType;
};

}

#endif // CONTENT_MLP_BOARD_H
