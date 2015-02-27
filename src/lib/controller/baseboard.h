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
    struct OLOLORD_EXPORT BanLevel
    {
        int level;
        std::string description;
    };
    struct OLOLORD_EXPORT File
    {
        std::string size;
        int sizeX;
        int sizeY;
        std::string sourceName;
        std::string thumbName;
    };
    struct OLOLORD_EXPORT Post
    {
        bool bannedFor;
        std::string cityName;
        std::string countryName;
        std::string dateTime;
        std::string email;
        std::list<File> files;
        std::string flagName;
        bool hidden;
        std::string ip;
        std::string name;
        std::string nameRaw;
        unsigned long long number;
        std::string rawPostText;
        bool showRegistered;
        std::string subject;
        bool subjectIsRaw;
        std::string text;
        unsigned long long threadNumber;
        std::string tripcode;
        bool showTripcode;
    };
public:
    std::string action;
    std::string ajaxErrorText;
    std::list<AbstractBoard::BoardInfo> availableBoards;
    std::string banExpiresLabelText;
    std::string banLevelLabelText;
    std::list<BanLevel> banLevels;
    std::string bannedForText;
    std::string bannerFileName;
    std::string banReasonLabelText;
    std::string banUserText;
    std::string boardLabelText;
    std::string bumpLimitReachedText;
    bool captchaEnabled;
    std::string captchaKey;
    unsigned long long captchaQuota;
    std::string captchaQuotaText;
    std::string closedText;
    std::string closeThreadText;
    AbstractBoard::BoardInfo currentBoard;
    unsigned long long currentThread;
    std::string deletePostText;
    std::string deleteThreadText;
    std::string editPostText;
    std::string enterPasswordText;
    std::string enterPasswordTitle;
    std::string fixedText;
    std::string fixThreadText;
    std::string hidePostFormText;
    unsigned int maxEmailLength;
    unsigned int maxNameLength;
    unsigned int maxSubjectLength;
    unsigned int maxPasswordLength;
    bool moder;
    std::string noCaptchaText;
    std::string notLoggedInText;
    std::string openThreadText;
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
    std::string showHidePostText;
    std::string showPostFormText;
    bool showWhois;
    std::string supportedFileTypes;
    std::string toBottomText;
    std::string toThread;
    std::string toTopText;
    std::string unfixThreadText;
};

}

#endif // BASETHREAD_H
