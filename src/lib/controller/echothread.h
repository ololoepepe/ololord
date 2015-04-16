#ifndef CONTENT_ECHOTHREAD_H
#define CONTENT_ECHOTHREAD_H

#include "controller/thread.h"

#include "../global.h"

#include <string>

namespace Content
{

struct OLOLORD_EXPORT echoThread : public Thread
{
    unsigned int maxLinkLength;
    std::string postFormLabelLink;
    std::string threadLink;
};

}

#endif // CONTENT_ECHOTHREAD_H
