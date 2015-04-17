#ifndef CONTENT_RPGBOARD_H
#define CONTENT_RPGBOARD_H

#include "controller/board.h"

#include "../global.h"

#include <string>

namespace Content
{

struct OLOLORD_EXPORT rpgBoard : public Board
{
    std::string addVoteVariantText;
    std::string multipleVoteVariantsText;
    std::string removeVoteVariantText;
    std::string postFormLabelVote;
    std::string unvoteActionText;
    unsigned long long userIp;
    std::string voteActionText;
    std::string votedText;
    std::string voteTextText;
};

}

#endif // CONTENT_RPGBOARD_H
