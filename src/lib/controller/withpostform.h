#ifndef WITHPOSTFORM_H
#define WITHPOSTFORM_H

#include "board/abstractboard.h"
#include "../global.h"

#include <list>
#include <string>

struct OLOLORD_EXPORT WithPostForm
{
    bool captchaEnabled;
    std::string captchaKey;
    AbstractBoard::BoardInfo currentBoard;
    std::string postFormButtonSubmit;
    std::string postFormInputFile;
    std::string postFormInputText;
    std::string postFormInputTextPlaceholder;
    std::string postFormLabelCaptcha;
    std::string postFormLabelEmail;
    std::string postFormLabelName;
    std::string postFormLabelPassword;
    std::string postFormLabelSubject;
};

#endif // WITHPOSTFORM_H
