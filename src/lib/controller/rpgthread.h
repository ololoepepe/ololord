#ifndef CONTENT_RPGTHREAD_H
#define CONTENT_RPGTHREAD_H

#include "controller/thread.h"

#include "../global.h"

#include <string>

namespace Content
{

struct OLOLORD_EXPORT rpgThread : public Thread
{
    std::string addVoteVariantText;
    std::string multipleVoteVariantsText;
    std::string removeVoteVariantText;
    std::string postFormLabelVote;
    unsigned long long userIp;
    bool voteEnabled;
    std::string voteText;
};

}

#endif // CONTENT_RPGTHREAD_H
