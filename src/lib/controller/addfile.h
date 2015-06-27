#ifndef CONTENT_ADDFILE_H
#define CONTENT_ADDFILE_H

#include "controller/base.h"

#include "../global.h"

#include <cppcms/view.h>

#include <string>

namespace Content
{

struct OLOLORD_EXPORT AddFile : public Base
{
    std::string currentBoardName;
    unsigned int maxFileCount;
    unsigned long long postNumber;
    std::string supportedFileTypes;
};

}

#endif // CONTENT_ADDFILE_H
