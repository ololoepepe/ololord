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
    std::string cspoilerText;
    std::string cspoilerTitle;
    std::string doubleHyphen;
    std::string emDash;
    std::string enDash;
    std::string italics;
    std::string linkMarkup;
    std::string listItem1;
    std::string listItem2;
    std::string listItem3;
    std::string listMarkup;
    std::string monospace;
    std::string monospaceEscaped;
    std::string nomarkup;
    std::string nomarkupEscaped;
    std::string postBoardLinkDescription;
    std::string postLinkDescription;
    std::string preformattedText;
    std::string quadripleHyphen;
    std::string quotation;
    std::string replacementMarkup;
    std::string spoiler;
    std::string strikedoutText;
    std::string strikedoutTextWakaba;
    std::string strikedoutWord1;
    std::string strikedoutWord2;
    std::string subscript;
    std::string superscript;
    std::string tooltip;
    std::string tooltipText;
    std::string underlinedText;
};

}

#endif // MARKUP_H
