#ifndef HELPERTHREAD_H
#define HELPERTHREAD_H

#include "../global.h"

#include <list>
#include <string>

struct OLOLORD_EXPORT HelperFile
{
    std::string size;
    std::string sourceName;
    std::string thumbName;
};

struct OLOLORD_EXPORT HelperPost
{
    bool bannedFor;
    std::string dateTime;
    std::string email;
    std::list<HelperFile> files;
    std::string name;
    unsigned long long number;
    std::string subject;
    std::string text;
};

struct OLOLORD_EXPORT HelperThread
{
    unsigned int bumpLimit;
    bool fixed;
    HelperPost opPost;
    std::list<HelperPost> lastPosts;
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
};

#endif // HELPERTHREAD_H
