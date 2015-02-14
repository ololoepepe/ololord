#ifndef CONTENT_ERROR_H
#define CONTENT_ERROR_H

#include "controller/base.h"

#include "../global.h"

#include <string>

namespace Content
{

struct OLOLORD_EXPORT Error : public Base
{
    std::string errorDescription;
    std::string errorMessage;
};

}

#endif // CONTENT_ERROR_H
