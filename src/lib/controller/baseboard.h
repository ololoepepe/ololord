#ifndef BASETHREAD_H
#define BASETHREAD_H

#include "controller/base.h"

#include "../global.h"

#include <list>
#include <string>

namespace Content
{

struct OLOLORD_EXPORT BaseBoard : public Base
{
    struct OLOLORD_EXPORT File
    {
        std::string size;
        std::string sourceName;
        std::string thumbName;
    };
    struct OLOLORD_EXPORT Post
    {
        bool bannedFor;
        std::string dateTime;
        std::string email;
        std::list<File> files;
        std::string name;
        std::string nameRaw;
        unsigned long long number;
        bool showRegistered;
        std::string subject;
        std::string text;
        std::string tripcode;
    };
public:
    std::string action;
    std::string ajaxErrorText;
    std::string bannedForText;
    std::string bannerFileName;
    std::string bumpLimitReachedText;
    bool captchaEnabled;
    std::string captchaKey;
    std::string closedText;
    AbstractBoard::BoardInfo currentBoard;
    unsigned long long currentThread;
    std::string deletePostText;
    std::string deleteThreadText;
    std::string enterPasswordText;
    std::string fixedText;
    std::string hidePostFormText;
    bool moder;
    std::string notLoggedInText;
    std::string postFormButtonSubmit;
    std::string postFormInputFile;
    std::string postFormInputText;
    std::string postFormInputTextPlaceholder;
    std::string postFormLabelCaptcha;
    std::string postFormLabelEmail;
    std::string postFormLabelName;
    std::string postFormLabelPassword;
    std::string postFormLabelSubject;
    std::string postingDisabledText;
    bool postingEnabled;
    std::string postLimitReachedText;
    std::string registeredText;
    std::string showPostFormText;
};

}

#endif // BASETHREAD_H
