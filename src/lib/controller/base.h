#ifndef CONTENT_BASE_H
#define CONTENT_BASE_H

#include "../global.h"
#include "board/abstractboard.h"

#include <cppcms/view.h>

#include <list>
#include <string>

namespace Content
{

struct OLOLORD_EXPORT Base : public cppcms::base_content
{
    struct CaptchaEngine
    {
        std::string id;
        std::string title;
    };
    struct Locale
    {
        std::string country;
        std::string language;
        std::string name;
    public:
        bool operator ==(const Locale &other) const
        {
            return name == other.name;
        }
    };
    struct Style
    {
        std::string name;
        std::string title;
    };
public:
    std::string allBoardsText;
    AbstractBoard::BoardInfoList boards;
    std::string cancelButtonText;
    std::list<CaptchaEngine> captchaEngines;
    std::string captchaLabelText;
    std::string captchaLabelWarningText;
    std::string closeButtonText;
    std::string confirmButtonText;
    CaptchaEngine currentCaptchaEngine;
    Locale currentLocale;
    std::string currentTime;
    std::string customFooterContent;
    std::string customHeaderContent;
    std::string favoriteThreadsText;
    std::string localeLabelText;
    std::list<Locale> locales;
    bool loggedIn;
    std::string loginButtonText;
    std::string loginLabelText;
    std::string loginMessageOk;
    std::string loginMessageWarning;
    std::string loginPlaceholderText;
    unsigned int maxSearchQueryLength;
    std::string pageTitle;
    std::string quickReplyActionAppendPostText;
    std::string quickReplyActionDoNothingText;
    std::string quickReplyActionGotoThreadText;
    std::string quickReplyActionLabelText;
    std::string removeFromFavoritesText;
    std::string searchButtonText;
    std::string searchInputPlaceholder;
    std::string settingsButtonText;
    std::string settingsDialogTitle;
    std::string showFavoriteText;
    std::string showPasswordText;
    std::string siteDomain;
    std::string sitePathPrefix;
    std::string siteProtocol;
    Style style;
    std::string styleLabelText;
    std::list<Style> styles;
    std::string timeLabelText;
    std::string timeLocalText;
    std::string timeServerText;
    std::string toHomePageText;
    std::string toPlaylistPageText;
    std::string toMarkupPageText;
};

}

#endif // CONTENT_BASE_H
