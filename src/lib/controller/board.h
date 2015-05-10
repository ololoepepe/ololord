#ifndef CONTENT_BOARD_H
#define CONTENT_BOARD_H

#include "controller/baseboard.h"

#include "../global.h"

#include <list>
#include <string>

namespace Content
{

struct OLOLORD_EXPORT Board : public BaseBoard
{
    struct OLOLORD_EXPORT Thread
    {
        unsigned int bumpLimit;
        bool closed;
        bool fixed;
        Post opPost;
        std::list<Post> lastPosts;
        unsigned int postCount;
        bool postingEnabled;
        unsigned int postLimit;
    public:
        bool bumpLimitReached()
        {
            return bumpLimit && (postCount >= bumpLimit);
        }
        unsigned int omittedPosts()
        {
            return (postCount > 4) ? (postCount - 4) : 0;
        }
        bool postLimitReached()
        {
            return postLimit && (postCount >= postLimit);
        }
        Thread *self()
        {
            return this;
        }
    };
public:
    typedef Thread * ThreadPointer;
public:
    std::string boardRulesLinkText;
    unsigned int currentPage;
    std::string omittedPostsText;
    std::list<unsigned int> pages;
    std::list<Thread> threads;
    std::string toNextPageText;
    std::string toPreviousPageText;
public:
    unsigned int nextPage() const
    {
        return currentPage + 1;
    }
    unsigned int previousPage() const
    {
        return currentPage ? (currentPage - 1) : 0;
    }
};

}

#endif // CONTENT_BOARD_H
