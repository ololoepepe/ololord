#ifndef CONTENT_THREAD_H
#define CONTENT_THREAD_H

#include "controller/helperthread.h"
#include "controller/withbanner.h"
#include "controller/withbase.h"
#include "controller/withnavbar.h"
#include "controller/withpostform.h"
#include "controller/withposts.h"
#include "controller/withsettings.h"
#include "../global.h"

#include <cppcms/view.h>

#include <list>
#include <string>

namespace Content
{

struct OLOLORD_EXPORT Thread : public cppcms::base_content, public WithBase, public WithNavbar, public WithBanner,
        public WithPostForm, public WithPosts, public WithSettings
{
    unsigned int bumpLimit;
    unsigned long long currentThread;
    bool fixed;
    std::string hidePostFormText;
    std::string hideSearchFormText;
    HelperPost opPost;
    std::string pageTitle;
    std::string postingDisabledText;
    bool postingEnabled;
    unsigned int postLimit;
    std::string postLimitReachedText;
    std::list<HelperPost> posts;
    std::string showPostFormText;
    std::string showSearchFormText;
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
