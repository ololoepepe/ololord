#ifndef CONTENT_PRTHREAD_H
#define CONTENT_PRTHREAD_H

#include "controller/thread.h"

#include "../global.h"

#include <string>

namespace Content
{

struct OLOLORD_EXPORT prThread : public Thread
{
    std::string codechaPublicKey;
};

}

#endif // CONTENT_PRTHREAD_H
