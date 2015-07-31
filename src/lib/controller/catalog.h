#ifndef CONTENT_CATALOG_H
#define CONTENT_CATALOG_H

#include "controller/baseboard.h"

#include "../global.h"

#include <list>
#include <string>

namespace Content
{

struct OLOLORD_EXPORT Catalog : public BaseBoard
{
    struct OLOLORD_EXPORT Thread
    {
        Post opPost;
        unsigned int replyCount;
    public:
        Thread *self()
        {
            return this;
        }
    };
public:
    typedef Thread * ThreadPointer;
public:
    std::string replyCountLabelText;
    std::string sortingModeBumpsLabelText;
    std::string sortingModeDateLabelText;
    std::string sortingModeLabelText;
    std::list<Thread> threads;
};

}

#endif // CONTENT_CATALOG_H
