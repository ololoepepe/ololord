#ifndef CONTENT_FAQ_H
#define CONTENT_FAQ_H

#include "controller/base.h"

#include "../global.h"

#include <string>

namespace Content
{

struct OLOLORD_EXPORT Faq : public Base
{
public:
    std::string custom;
};

}

#endif // CONTENT_FAQ_H
