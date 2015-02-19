#ifndef CONTENT_PRBOARD_H
#define CONTENT_PRBOARD_H

#include "controller/board.h"

#include "../global.h"

#include <string>

namespace Content
{

struct OLOLORD_EXPORT prBoard : public Board
{
    std::string codechaPublicKey;
};

}

#endif // CONTENT_PRBOARD_H
