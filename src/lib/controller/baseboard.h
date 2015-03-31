#ifndef BASETHREAD_H
#define BASETHREAD_H

#include "controller/base.h"

#include "../global.h"
#include "tools.h"

#include <QString>
#include <QStringList>
#include <QVariant>

#include <list>
#include <string>

namespace Content
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
    int thumbSizeX;
    int thumbSizeY;
    std::string type;
public:
    std::string specialThumbName() const
    {
        return Tools::toStd(Tools::fromStd(thumbName).replace('/', "_"));
    }
};

struct OLOLORD_EXPORT Post
{
    struct Ref
    {
        std::string boardName;
        unsigned long long postNumber;
        unsigned long long threadNumber;
    };
public:
    bool bannedFor;
    std::string cityName;
    bool closed;
    std::string countryName;
    std::string dateTime;
    bool draft;
    std::string email;
    std::list<File> files;
    bool fixed;
    std::string flagName;
    bool hidden;
    std::string ip;
    std::string modificationDateTime;
    std::string name;
    std::string nameRaw;
    unsigned long long number;
    std::string rawName;
    bool rawHtml;
    std::string rawPostText;
    std::string rawSubject;
    std::list<Ref> referencedBy;
    bool showRegistered;
    std::string subject;
    bool subjectIsRaw;
    std::string text;
    unsigned long long threadNumber;
    std::string tripcode;
    bool showTripcode;
    QVariant userData;
};

struct OLOLORD_EXPORT BaseBoard : public Base
{
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
    std::string bytesText;
    bool captchaEnabled;
    std::string captchaKey;
    unsigned long long captchaQuota;
    std::string captchaQuotaText;
    std::string closedText;
    std::string closeThreadText;
    std::string complainMessage;
    std::string complainText;
    AbstractBoard::BoardInfo currentBoard;
    unsigned long long currentThread;
    std::string deletePostText;
    std::string deleteThreadText;
    std::string downloadThreadText;
    bool draftsEnabled;
    std::string editPostText;
    std::string enterPasswordText;
    std::string enterPasswordTitle;
    std::string findSourceWithGoogleText;
    std::string findSourceWithIqdbText;
    std::string fixedText;
    std::string fixThreadText;
    std::string hidePostFormText;
    std::string kilobytesText;
    unsigned int maxEmailLength;
    unsigned int maxFileCount;
    unsigned int maxNameLength;
    unsigned int maxSubjectLength;
    unsigned int maxPasswordLength;
    std::string megabytesText;
    int moder;
    std::string modificationDateTimeText;
    std::string noCaptchaText;
    std::string notLoggedInText;
    std::string openThreadText;
    std::string postFormButtonSubmit;
    std::string postFormInputFile;
    std::string postFormLabelCaptcha;
    std::string postFormLabelDraft;
    std::string postFormLabelEmail;
    std::string postFormLabelName;
    std::string postFormLabelPassword;
    std::string postFormLabelRaw;
    std::string postFormLabelSubject;
    std::string postFormLabelText;
    std::list<std::string> postformRules;
    std::string postFormTextPlaceholder;
    std::string postingDisabledText;
    bool postingEnabled;
    std::string postLimitReachedText;
    std::string referencedByText;
    std::string registeredText;
    std::string removeFileText;
    std::string selectFileText;
    std::string showHidePostText;
    std::string showPostFormText;
    bool showWhois;
    std::string supportedFileTypes;
    std::string toBottomText;
    std::string toThread;
    std::string toTopText;
    std::string unfixThreadText;
public:
    static bool isAudioType(const std::string &mimeType)
    {
        return Tools::isAudioType(Tools::fromStd(mimeType));
    }
    static bool isImageType(const std::string &mimeType)
    {
        return Tools::isImageType(Tools::fromStd(mimeType));
    }
    static bool isVideoType(const std::string &mimeType)
    {
        return Tools::isVideoType(Tools::fromStd(mimeType));
    }
    static bool isSpecialThumbName(const std::string &tn)
    {
        return Tools::isSpecialThumbName(Tools::fromStd(tn));
    }
};

}

#endif // BASETHREAD_H
