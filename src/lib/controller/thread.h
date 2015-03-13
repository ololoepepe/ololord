#ifndef CONTENT_THREAD_H
#define CONTENT_THREAD_H

#include "controller/baseboard.h"

#include "../global.h"

#include <list>
#include <string>

namespace Content
{

struct OLOLORD_EXPORT Thread : public BaseBoard
{
    bool autoUpdateEnabled;
    std::string autoUpdateText;
    std::string backText;
    unsigned int bumpLimit;
    bool closed;
    bool fixed;
    bool hidden;
    unsigned long long id;
    std::string newPostsText;
    std::string noNewPostsText;
    unsigned long long number;
    Post opPost;
    unsigned int postLimit;
    std::list<Post> posts;
    std::string updateThreadText;
public:
    bool bumpLimitReached()
    {
        return bumpLimit && ((posts.size() + 1) >= bumpLimit);
    }
    bool postLimitReached()
    {
        return postLimit && ((posts.size() + 1) >= postLimit);
    }
};

}

#endif // CONTENT_THREAD_H
