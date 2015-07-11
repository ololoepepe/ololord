#ifndef CONTENT_RPGEDITPOST_H
#define CONTENT_RPGEDITPOST_H

#include "controller/editpost.h"

#include "../global.h"

#include <cppcms/view.h>

#include <list>
#include <string>

namespace Content
{

struct OLOLORD_EXPORT rpgEditPost : public EditPost
{
    struct VoteVariant
    {
        std::string id;
        std::string text;
    };
public:
    bool multiple;
    std::string multipleVoteVariantsText;
    std::string postFormLabelVote;
    std::string voteText;
    std::string voteTextText;
    std::list<VoteVariant> voteVariants;
};

}

#endif // CONTENT_RPGEDITPOST_H
