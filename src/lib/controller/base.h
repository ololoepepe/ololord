#ifndef CONTENT_BASE_H
#define CONTENT_BASE_H

#include "../global.h"
#include "board/abstractboard.h"

#include <cppcms/view.h>

#include <list>
#include <set>
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
    struct Mode
    {
        std::string name;
        std::string title;
    };
    struct Style
    {
        std::string name;
        std::string title;
    };
public:
    std::string addToFavoritesOnReplyLabelText;
    std::string allBoardsText;
    std::string autoUpdateIntervalLabelText;
    std::string autoUpdateThreadsByDefaultLabelText;
    AbstractBoard::BoardInfoList boards;
    std::string cancelButtonText;
    std::list<CaptchaEngine> captchaEngines;
    std::string captchaLabelText;
    std::string captchaLabelWarningText;
    std::string checkFileExistenceLabelText;
    std::string closeButtonText;
    std::string confirmButtonText;
    CaptchaEngine currentCaptchaEngine;
    Locale currentLocale;
    std::string currentTime;
    std::string customFooterContent;
    std::string customHeaderContent;
    std::string defaultAudioVideoVolumeLabelText;
    std::string deviceType;
    bool draftsByDefault;
    std::string draftsByDefaultLabelText;
    std::string editHotkeysText;
    std::string editSpellsText;
    std::string editUserCssText;
    std::string error413Text;
    std::string favoriteThreadsText;
    std::string filesTabText;
    std::string framedVersionText;
    std::string generalSettingsLegendText;
    std::set<std::string> hiddenBoards;
    std::string hiddenBoardsLabelText;
    std::string hiddenPostListText;
    std::string hidePostformMarkupLabelText;
    bool hidePostformRules;
    std::string hidePostformRulesLabelText;
    std::string hideTripcodesLabelText;
    std::string hideUserNamesLabelText;
    std::string hidingTabText;
    std::string hotkeyExpandImageLabelText;
    std::string hotkeyExpandThreadLabelText;
    std::string hotkeyGoToThreadLabelText;
    std::string hotkeyHidePostLabelText;
    std::string hotkeyMarkupBoldLabelText;
    std::string hotkeyMarkupCodeLabelText;
    std::string hotkeyMarkupItalicsLabelText;
    std::string hotkeyMarkupQutationLabelText;
    std::string hotkeyMarkupSpoilerLabelText;
    std::string hotkeyMarkupStrikedOutLabelText;
    std::string hotkeyMarkupUnderlinedLabelText;
    std::string hotkeyNextPageImageLabelText;
    std::string hotkeyNextPostLabelText;
    std::string hotkeyNextThreadPostLabelText;
    std::string hotkeyPreviousPageImageLabelText;
    std::string hotkeyPreviousPostLabelText;
    std::string hotkeyPreviousThreadPostLabelText;
    std::string hotkeyQuickReplyLabelText;
    std::string hotkeyShowFavoritesLabelText;
    std::string hotkeyShowSettingsLabelText;
    std::string hotkeysLabelText;
    std::string hotkeySubmitReplyLabelText;
    std::string hotkeyUpdateThreadLabelText;
    std::string imageZoomSensitivityLabelText;
    std::string leafThroughImagesOnlyLabelText;
    std::string localeLabelText;
    std::list<Locale> locales;
    bool loggedIn;
    std::string loginButtonText;
    std::string loginIconName;
    std::string loginLabelText;
    std::string loginMessageText;
    std::string loginPlaceholderText;
    std::string loginSystemDescriptionText;
    std::string loopAudioVideoLabelText;
    int maxAllowedRating;
    std::string maxAllowedRatingLabelText;
    unsigned int maxSearchQueryLength;
    Mode mode;
    std::string modeLabelText;
    std::list<Mode> modes;
    std::string mumWatchingText;
    std::string otherTabText;
    std::string pageTitle;
    std::string path;
    std::string playAudioVideoImmediatelyLabelText;
    std::string postformTabText;
    std::string postsTabText;
    std::string quickReplyActionAppendPostText;
    std::string quickReplyActionDoNothingText;
    std::string quickReplyActionGotoThreadText;
    std::string quickReplyActionLabelText;
    std::string ratingTooltip;
    std::string rememberAudioVideoVolumeLabelText;
    std::string removeFromFavoritesText;
    std::string removeFromHiddenPostListText;
    std::string scriptSettingsLegendText;
    std::string searchButtonText;
    std::string searchInputPlaceholder;
    std::string settingsButtonText;
    std::string settingsDialogTitle;
    std::string showAttachedFilePreviewLabelText;
    std::string showAutoUpdateDesktopNotificationsLabelText;
    std::string showAutoUpdateTimerLabelText;
    std::string showFavoriteText;
    std::string showHiddenPostListText;
    std::string showLeafButtonsLabelText;
    std::string showNewPostsLabelText;
    std::string showPasswordText;
    std::string showYoutubeVideoTitleLabelText;
    std::string siteDomain;
    std::string sitePathPrefix;
    std::string siteProtocol;
    std::string spellsLabelText;
    std::string strikeOutHiddenPostLinksLabelText;
    std::string stripExifFromJpegLabelText;
    Style style;
    std::string styleLabelText;
    std::list<Style> styles;
    std::string timeLabelText;
    std::string timeLocalText;
    std::string timeServerText;
    int timeZoneOffset;
    std::string timeZoneOffsetLabelText;
    std::string toHomePageText;
    std::string toPlaylistPageText;
    std::string toMarkupPageText;
    std::string userCssLabelText;
};

}

#endif // CONTENT_BASE_H
