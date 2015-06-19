#ifndef MARKUP_H
#define MARKUP_H

#include "controller/base.h"

#include "../global.h"

#include <string>

namespace Content
{

struct OLOLORD_EXPORT Markup : public Base
{
    std::string basicMarkup;
    std::string boldText;
    std::string codeMarkup;
    std::string combinedText;
    std::string doubleMonospace;
    std::string italics;
    std::string listDescription;
    std::string listItem1;
    std::string listItem2;
    std::string listItem3;
    std::string listMarkup;
    std::string postBoardLinkDescription;
    std::string postLinkDescription;
    std::string preformattedText;
    std::string quotation;
    std::string singleMonospace;
    std::string spoiler;
    std::string strikedoutText;
    std::string strikedoutTextWakaba;
    std::string tooltip;
    std::string tooltipText;
    std::string underlinedText;
};

}

#endif // MARKUP_H
