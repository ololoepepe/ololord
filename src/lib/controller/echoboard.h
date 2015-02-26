#ifndef CONTENT_ECHOPOST_H
#define CONTENT_ECHOPOST_H

#include "controller/board.h"

#include "../global.h"

#include <string>

namespace Content
{

struct OLOLORD_EXPORT echoBoard : public Board
{
    unsigned int maxLinkLength;
    std::string postFormLabelLink;
};

}

#endif // CONTENT_ECHOPOST_H
