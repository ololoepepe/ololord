#ifndef CONTENT_BOARD_H
#define CONTENT_BOARD_H

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

struct OLOLORD_EXPORT Board : public cppcms::base_content, public WithBase, public WithNavbar, public WithBanner,
        public WithPostForm, public WithPosts, public WithSettings
{
public:
    std::string boardRulesLinkText;
    unsigned int currentPage;
    std::string hideSearchFormText;
    std::string hideThreadFormText;
    std::string omittedPostsText;
    std::list<unsigned int> pages;
    std::string pageTitle;
    std::string postingDisabledText;
    bool postingEnabled;
    std::string postLimitReachedText;
    std::string showSearchFormText;
    std::string showThreadFormText;
    std::list<HelperThread> threads;
    std::string toNextPageText;
    std::string toPreviousPageText;
    std::string toThread;
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
