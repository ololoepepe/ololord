#ifndef SEARCH_H
#define SEARCH_H

#include "controller/base.h"

#include "../global.h"

#include <list>
#include <string>

namespace Content
{

struct OLOLORD_EXPORT Search : public Base
{
    struct SearchResult
    {
        std::string boardName;
        unsigned int postNumber;
        std::string subject;
        std::string text;
        unsigned int threadNumber;
    };
public:
    bool error;
    std::string errorMessage;
    std::string errorDescription;
    std::string nothingFoundMessage;
    std::string query;
    std::string queryBoard;
    std::string resultsMessage;
    std::list<SearchResult> searchResults;
};

}

#endif // SEARCH_H
