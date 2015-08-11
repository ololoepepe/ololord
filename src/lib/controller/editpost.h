#ifndef CONTENT_EDITPOST_H
#define CONTENT_EDITPOST_H

#include "controller/base.h"

#include "../global.h"

#include <cppcms/view.h>

#include <list>
#include <string>

namespace Content
{

struct OLOLORD_EXPORT EditPost : public Base
{
    struct MarkupMode
    {
        std::string name;
        std::string title;
    };
public:
    std::string currentBoardName;
    MarkupMode currentMarkupMode;
    bool draft;
    bool draftsEnabled;
    std::string email;
    MarkupMode markupMode;
    std::list<MarkupMode> markupModes;
    unsigned int maxEmailLength;
    unsigned int maxNameLength;
    unsigned int maxSubjectLength;
    int moder;
    std::string name;
    std::string postFormLabelDraft;
    std::string postFormLabelEmail;
    std::string postFormLabelMarkupMode;
    std::string postFormLabelName;
    std::string postFormLabelRaw;
    std::string postFormLabelSubject;
    std::string postFormLabelText;
    std::string postFormTextPlaceholder;
    unsigned long long postNumber;
    bool raw;
    std::string subject;
    std::string text;
};

}

#endif // EDITPOST_H
