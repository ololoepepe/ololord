#include "controller.h"

#include "ban.h"
#include "base.h"
#include "baseboard.h"
#include "board/abstractboard.h"
#include "captcha/abstractcaptchaengine.h"
#include "database.h"
#include "error.h"
#include "ipban.h"
#include "notfound.h"
#include "settingslocker.h"
#include "tools.h"
#include "translator.h"

#include <BCoreApplication>
#include <BeQt>
#include <BTranslation>

#include <QChar>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QLocale>
#include <QMap>
#include <QMutex>
#include <QSettings>
#include <QString>
#include <QStringList>

#include <cppcms/application.h>
#include <cppcms/http_cookie.h>
#include <cppcms/http_request.h>
#include <cppcms/http_response.h>
#include <cppcms/json.h>

#include <list>
#include <string>

namespace Controller
{

static QMutex localeMutex(QMutex::Recursive);

static std::string speedString(const AbstractBoard::PostingSpeed &s, double duptime)
{
    double d = double(s.postCount) / duptime;
    QString ss = QString::number(d, 'f', 1);
    return Tools::toStd((ss.split('.').last() != "0") ? ss : ss.split('.').first());
}

static std::string zeroSpeedString(const AbstractBoard::PostingSpeed &s, const std::string &nonZero, const QLocale &l)
{
    if (s.postCount && s.uptimeMsecs)
        return "1 " + nonZero;
    else
        return "0 " + TranslatorStd(l).translate("zeroSpeedString", "post(s) per hour.", "postingSpeed");
}

static Content::Base::Locale toWithLocale(const QLocale &l)
{
    Content::Base::Locale ll;
    ll.country = Tools::toStd(l.nativeCountryName());
    ll.name = Tools::toStd(l.name());
    ll.language = Tools::toStd(l.nativeLanguageName());
    return ll;
}

void initBase(Content::Base &c, const cppcms::http::request &req, const QString &pageTitle)
{
    typedef std::list<Content::Base::Locale> LocaleList;
    localeMutex.lock();
    init_once(LocaleList, locales, LocaleList()) {
        foreach (const QString &path, BCoreApplication::locations(BCoreApplication::TranslationsPath)) {
            foreach (const QString &fn, QDir(path).entryList(QStringList() << "ololord_*.qm", QDir::Files)) {
                QString ln = fn.mid(8);
                ln.remove(ln.length() - 3, 3);
                QLocale ll(ln);
                if (QLocale::c() == ll)
                    continue;
                Content::Base::Locale lll = toWithLocale(ll);
                if (std::find(locales.begin(), locales.end(), lll) != locales.end())
                    continue;
                locales.push_back(lll);
            }
        }
        locales.push_back(toWithLocale(QLocale("en_US")));
    }
    typedef QMap<QString, BTranslation> TranslationMap;
    init_once(TranslationMap, styles, TranslationMap()) {
        styles.insert("photon", BTranslation::translate("initBase", "Photon", "style title"));
        styles.insert("futaba", BTranslation::translate("initBase", "Futaba", "style title"));
    }
    init_once(TranslationMap, modes, TranslationMap()) {
        modes.insert("normal", BTranslation::translate("initBase", "Normal", "mode title"));
        modes.insert("ascetic", BTranslation::translate("initBase", "Ascetic", "mode title"));
    }
    localeMutex.unlock();
    TranslatorStd ts(req);
    c.allBoardsText = ts.translate("initBase", "All boards", "allBoardsText");
    c.boards = AbstractBoard::boardInfos(ts.locale(), false);
    c.captchaLabelText = ts.translate("initBase", "Captcha:", "captchaLabelText");
    c.captchaLabelWarningText = ts.translate("initBase", "This option may be ignored on some boards",
                                             "captchaLabelWarningText");
    AbstractCaptchaEngine::LockingWrapper ce = AbstractCaptchaEngine::engine(Tools::cookieValue(req, "captchaEngine"));
    if (!ce.isNull()) {
        c.currentCaptchaEngine.id = Tools::toStd(ce->id());
        c.currentCaptchaEngine.title = Tools::toStd(ce->title(ts.locale()));
    }
    AbstractCaptchaEngine::EngineInfoList eilist = AbstractCaptchaEngine::engineInfos(ts.locale());
    foreach (const AbstractCaptchaEngine::EngineInfo &inf, eilist) {
        Content::BaseBoard::CaptchaEngine e;
        e.id = inf.id;
        e.title = inf.title;
        c.captchaEngines.push_back(e);
        if (ce.isNull() && inf.id == "google-recaptcha") {
            c.currentCaptchaEngine.id = inf.id;
            c.currentCaptchaEngine.title = inf.title;
        }
    }
    if (ce.isNull() && c.currentCaptchaEngine.id.empty() && !eilist.isEmpty()) {
        c.currentCaptchaEngine.id = eilist.first().id;
        c.currentCaptchaEngine.title = eilist.first().title;
    }
    c.cancelButtonText = ts.translate("initBase", "Cancel", "cancelButtonText");
    c.closeButtonText = ts.translate("initBase", "Close", "closeButtonText");
    c.confirmButtonText = ts.translate("initBase", "Confirm", "confirmButtonText");
    c.currentLocale = toWithLocale(ts.locale());
    cppcms::http::request *mreq = const_cast<cppcms::http::request *>(&req);
    c.currentTime = mreq->cookie_by_name("time").value();
    c.customFooterContent = Tools::toStd(Tools::customContent("footer", ts.locale()));
    c.customHeaderContent = Tools::toStd(Tools::customContent("header", ts.locale()));
    c.error413Text = ts.translate("initBase", "Request entity too large", "error413Text");
    c.favoriteThreadsText = ts.translate("initBase", "Favorite threads", "favoriteThreadsText");
    c.localeLabelText = "Language:";
    c.locales = locales;
    c.loggedIn = !Tools::hashpassString(req).isEmpty();
    c.loginButtonText = c.loggedIn ? ts.translate("initBase", "Logout", "loginButtonText")
                                   : ts.translate("initBase", "Login", "loginButtonText");
    c.loginLabelText = ts.translate("initBase", "Login:", "loginLabelText");
    if (c.loggedIn) {
        int lvl = Database::registeredUserLevel(req);
        if (lvl < 0) {
            c.loginMessageWarning = ts.translate("initBase", "Logged in, but not registered", "loginMessageWarning");
        } else {
            c.loginMessageOk = ts.translate("initBase", "Registered and logged in", "loginMessageOk");
            if (lvl >= RegisteredUser::AdminLevel)
                c.loginMessageOk += " (" + ts.translate("initBase", "admin", "loginMessageOk") + ")";
            else if (lvl >= RegisteredUser::ModerLevel)
                c.loginMessageOk += " (" + ts.translate("initBase", "moder", "loginMessageOk") + ")";
            else if (lvl >= RegisteredUser::ModerLevel)
                c.loginMessageOk += " (" + ts.translate("initBase", "user", "loginMessageOk") + ")";
        }
    }
    c.loginPlaceholderText = ts.translate("initBase", "Password/hashpass", "PlaceholderText");
    c.maxSearchQueryLength = 150;
    c.mode.name = Tools::toStd(Tools::cookieValue(req, "mode"));
    if (c.mode.name.empty())
        c.mode.name = "normal";
    c.mode.title = Tools::toStd(modes.value(Tools::fromStd(c.mode.name)).translate());
    c.modeLabelText = ts.translate("initBase", "Mode:", "modeLabelText");
    foreach (const QString &s, modes.keys()) {
        Content::Base::Mode m;
        m.name = Tools::toStd(s);
        m.title = Tools::toStd(modes.value(s).translate());
        c.modes.push_back(m);
    }
    c.pageTitle = Tools::toStd(pageTitle);
    c.quickReplyActionAppendPostText = ts.translate("initBase", "Appends a new post",
                                                    "quickReplyActionAppendPostText");
    c.quickReplyActionDoNothingText = ts.translate("initBase", "Leaves page unmodified",
                                                   "quickReplyActionDoNothingText");
    c.quickReplyActionGotoThreadText = ts.translate("initBase", "Redirects to thread",
                                                    "quickReplyActionGotoThreadText");
    c.quickReplyActionLabelText = ts.translate("initBase", "Quick reply outside thread:", "quickReplyActionLabelText");
    c.removeFromFavoritesText = ts.translate("initBase", "Remove from favorites", "removeFromFavoritesText");
    SettingsLocker s;
    c.searchButtonText = ts.translate("initBase", "Search", "searchButtonText");
    c.searchInputPlaceholder = ts.translate("initBase", "Search: possible +required -excluded",
                                            "searchInputPlaceholder");
    c.settingsButtonText = ts.translate("initBase", "Settings", "settingsButtonText");
    c.settingsDialogTitle = ts.translate("initBase", "Settings", "settingsDialogTitle");
    c.showFavoriteText = ts.translate("initBase", "Favorites", "showFavoriteText");
    c.showPasswordText = ts.translate("initBase", "Show password", "showPasswordText");
    c.siteDomain = Tools::toStd(s->value("Site/domain").toString());
    c.sitePathPrefix = Tools::toStd(s->value("Site/path_prefix").toString());
    c.siteProtocol = Tools::toStd(s->value("Site/protocol").toString());
    if (c.siteProtocol.empty())
        c.siteProtocol = "http";
    c.style.name = Tools::toStd(Tools::cookieValue(req, "style"));
    if (c.style.name.empty())
        c.style.name = "photon";
    c.style.title = Tools::toStd(styles.value(Tools::fromStd(c.style.name)).translate());
    c.styleLabelText = ts.translate("initBase", "Style:", "styleLabelText");
    foreach (const QString &s, styles.keys()) {
        Content::Base::Style st;
        st.name = Tools::toStd(s);
        st.title = Tools::toStd(styles.value(s).translate());
        c.styles.push_back(st);
    }
    c.timeLabelText = ts.translate("initBase", "Time:", "timeLabelText");
    c.timeLocalText = ts.translate("initBase", "Local", "timeLocalText");
    c.timeServerText = ts.translate("initBase", "Server", "timeServerText");
    c.toHomePageText = ts.translate("initBase", "Home", "toHomePageText");
    c.toPlaylistPageText = ts.translate("initBase", "Playlist", "toPlaylistPageText");
    c.toMarkupPageText = ts.translate("initBase", "Markup", "toMarkupPageText");
}

bool initBaseBoard(Content::BaseBoard &c, const cppcms::http::request &req, const AbstractBoard *board,
                   bool postingEnabled, const QString &pageTitle, quint64 currentThread)
{
    if (!board)
        return false;
    TranslatorStd ts(req);
    TranslatorQt tq(req);
    initBase(c, req, pageTitle);
    if (c.pageTitle.empty() && currentThread)
        c.pageTitle = Tools::toStd(board->title(ts.locale()) + " - " + QString::number(currentThread));
    QStringList userBoards = Database::registeredUserBoards(req);
    if (userBoards.size() == 1 && userBoards.first() == "*")
        userBoards << AbstractBoard::boardNames();
    foreach (const QString &s, userBoards) {
        AbstractBoard::BoardInfo inf;
        inf.name = Tools::toStd(s);
        AbstractBoard::LockingWrapper b = AbstractBoard::board(s);
        inf.title = Tools::toStd(b ? b->title(tq.locale()) : tq.translate("initBaseBoard", "All boards", "boardName"));
        c.availableBoards.push_back(inf);
    }
    c.action = currentThread ? "create_post" : "create_thread";
    c.addFileText = ts.translate("initBaseBoard", "Add file", "addFileText");
    c.addToPlaylistText = ts.translate("initBaseBoard", "Add to playlist", "addToPlaylistText");
    c.addThreadToFavoritesText = ts.translate("initBaseBoard", "Add thread to favorites", "addThreadToFavoritesText");
    c.ajaxErrorText = ts.translate("initBaseBoard", "AJAX request returned status", "ajaxErrorText");
    c.audioTagAlbumText = ts.translate("initBaseBoard", "Album:", "audioTagAlbumText");
    c.audioTagArtistText = ts.translate("initBaseBoard", "Artist:", "audioTagArtistText");
    c.audioTagTitleText = ts.translate("initBaseBoard", "Title:", "audioTagTitleText");
    c.audioTagYearText = ts.translate("initBaseBoard", "Year:", "audioTagYearText");
    c.banExpiresLabelText = ts.translate("initBaseBoard", "Expiration time:", "banExpiresLabelText");
    c.banLevelLabelText = ts.translate("initBaseBoard", "Level:", "banLevelLabelText");
    Content::BanLevel bl;
    bl.level = 0;
    bl.description = ts.translate("initBaseBoard", "Not banned", "banLevelDesctiption");
    c.banLevels.push_back(bl);
    bl.level = 1;
    bl.description = ts.translate("initBaseBoard", "Posting prohibited", "banLevelDesctiption");
    c.banLevels.push_back(bl);
    bl.level = 10;
    bl.description = ts.translate("initBaseBoard", "Posting and reading prohibited", "banLevelDesctiption");
    c.banLevels.push_back(bl);
    c.bannedForText = ts.translate("initBaseBoard", "User was banned for this post", "bannedForText");
    c.bannerFileName = Tools::toStd(board->bannerFileName());
    c.banReasonLabelText = ts.translate("initBaseBoard", "Reason:", "banReasonLabelText");
    c.banUserText = ts.translate("initBaseBoard", "Ban user", "banUserText");
    c.boardLabelText = ts.translate("initBaseBoard", "Board:", "boardLabelText");
    c.bytesText = ts.translate("initBaseBoard", "Byte(s)", "bytesText");
    c.bumpLimitReachedText = ts.translate("initBaseBoard", "Bump limit reached", "bumpLimitReachedText");
    QString ip = Tools::userIp(req);
    c.captchaEnabled = Tools::captchaEnabled(board->name());
    QStringList supportedCaptchaEngines = board->supportedCaptchaEngines().split(',');
    if (supportedCaptchaEngines.isEmpty())
        return false;
    QString ceid = Tools::cookieValue(req, "captchaEngine");
    if (ceid.isEmpty() || !supportedCaptchaEngines.contains(ceid, Qt::CaseInsensitive)) {
        if (supportedCaptchaEngines.contains("google-recaptcha"))
            ceid = "google-recaptcha";
        else
            ceid = supportedCaptchaEngines.first();
    }
    AbstractCaptchaEngine::LockingWrapper ce = AbstractCaptchaEngine::engine(ceid);
    if (ce.isNull())
        return false;
    c.captchaHeaderHtml = Tools::toStd(ce->headerHtml());
    c.captchaScriptSource = Tools::toStd(ce->scriptSource());
    c.captchaWidgetHtml = Tools::toStd(ce->widgetHtml());
    c.captchaQuota = board->captchaQuota(ip);
    c.captchaQuotaText = ts.translate("initBaseBoard", "Posts without captcha left:", "captchaQuotaText");
    c.closedText = ts.translate("initBaseBoard", "The thread is closed", "closedText");
    c.closeThreadText = ts.translate("initBaseBoard", "Close thread", "closeThreadText");
    c.collapseVideoText = ts.translate("initBaseBoard", "Collapse video", "collapseVideoText");
    c.complainText = ts.translate("initBaseBoard", "Complain", "complainText");
    c.complainMessage = ts.translate("initBaseBoard", "Go complain to your mum, you whiner!", "complainMessage");
    c.currentBoard.name = Tools::toStd(board->name());
    c.currentBoard.title = Tools::toStd(board->title(ts.locale()));
    c.currentThread = currentThread;
    c.deleteFileText = ts.translate("initBaseBoard", "Delete file", "deleteFileText");
    c.deletePostText = ts.translate("initBaseBoard", "Delete post", "deletePostText");
    c.deleteThreadText = ts.translate("initBaseBoard", "Delete thread", "deleteThreadText");
    c.downloadThreadText = ts.translate("initBaseBoard", "Download all thread files as a .zip archive",
                                        "downloadThreadText");
    c.draftsEnabled = board->draftsEnabled();
    c.editAudioTagsText = ts.translate("initBaseBoard", "Edit audio file tags", "editAudioTagsText");
    c.editPostText = ts.translate("initBaseBoard", "Edit post", "editPostText");
    c.enterPasswordText = ts.translate("initBaseBoard", "If password is empty, current hashpass will be used",
                                       "enterPasswordText");
    c.enterPasswordTitle = ts.translate("initBaseBoard", "Enter password", "enterPasswordTitle");
    c.expandVideoText = ts.translate("initBaseBoard", "Expand video", "expandVideoText");
    c.fileExistsOnServerText = ts.translate("initBaseBoard", "This file exists on server. "
                                            "It will NOT be uploaded, but WILL be copied.", "fileExistsOnServerText");
    c.findSourceWithGoogleText = ts.translate("initBaseBoard", "Find source with Google", "findSourceWithGoogleText");
    c.findSourceWithIqdbText = ts.translate("initBaseBoard", "Find source with Iqdb", "findSourceWithIqdbText");
    c.fixedText = ts.translate("initBaseBoard", "Fixed", "fixedText");
    c.fixThreadText = ts.translate("initBaseBoard", "Fix thread", "fixThreadText");
    c.hidePostFormText = ts.translate("initBaseBoard", "Hide post form", "hidePostFormText");
    c.kilobytesText = ts.translate("initBaseBoard", "KB", "kilobytesText");
    c.maxEmailLength = Tools::maxInfo(Tools::MaxEmailFieldLength, board->name());
    c.maxFileCount = Tools::maxInfo(Tools::MaxFileCount, board->name());
    c.maxNameLength = Tools::maxInfo(Tools::MaxNameFieldLength, board->name());
    c.maxSubjectLength = Tools::maxInfo(Tools::MaxSubjectFieldLength, board->name());
    c.maxPasswordLength = Tools::maxInfo(Tools::MaxPasswordFieldLength, board->name());
    c.maxTextLength = Tools::maxInfo(Tools::MaxTextFieldLength, board->name());
    c.megabytesText = ts.translate("initBaseBoard", "MB", "megabytesText");
    c.moder = Database::registeredUserLevel(req) / 10;
    if (c.moder > 0) {
        QStringList boards = Database::registeredUserBoards(req);
        if (!boards.contains("*") && !boards.contains(board->name()))
            c.moder = 0;
    }
    c.modificationDateTimeText = ts.translate("initBaseBoard", "Last modified:", "modificationDateTimeText");
    c.noCaptchaText = ts.translate("initBaseBoard", "You don't have to enter captcha", "noCaptchaText");
    c.notLoggedInText = ts.translate("initBaseBoard", "You are not logged in!", "notLoggedInText");
    c.openThreadText = ts.translate("initBaseBoard", "Open thread", "openThreadText");
    c.postActionsText = ts.translate("initBaseBoard", "Post actions", "postActionsText");
    c.postFormButtonSubmit = ts.translate("initBaseBoard", "Send", "postFormButtonSubmit");
    c.postFormButtonSubmitSending = ts.translate("initBaseBoard", "Sending:", "postFormButtonSubmitSending");
    c.postFormButtonSubmitWaiting = ts.translate("initBaseBoard", "Waiting for reply...",
                                                 "postFormButtonSubmitWaiting");
    c.postFormInputFile = ts.translate("initBaseBoard", "File(s):", "postFormInputFile");
    SettingsLocker s;
    int maxText = s->value("Board/" + board->name() + "/max_text_length",
                           s->value("Board/max_text_length", 15000)).toInt();
    c.postFormTextPlaceholder = Tools::toStd(tq.translate("initBaseBoard", "Comment. Max length %1",
                                                           "postFormTextPlaceholder").arg(maxText));
    c.postFormLabelCaptcha = ts.translate("initBaseBoard", "Captcha:", "postFormLabelCaptcha");
    c.postFormLabelDraft = ts.translate("initBaseBoard", "Draft:", "postFormLabelDraft");
    c.postFormLabelEmail = ts.translate("initBaseBoard", "E-mail:", "postFormLabelEmail");
    c.postFormLabelName = ts.translate("initBaseBoard", "Name:", "postFormLabelName");
    c.postFormLabelPassword = ts.translate("initBaseBoard", "Password:", "postFormLabelPassword");
    c.postFormLabelRaw = ts.translate("initBaseBoard", "Raw HTML:", "postFormLabelRaw");
    c.postFormLabelSubject = ts.translate("initBaseBoard", "Subject:", "postFormLabelSubject");
    c.postFormLabelText = ts.translate("initBaseBoard", "Post:", "postFormLabelText");
    c.postFormLabelTripcode = ts.translate("initBaseBoard", "Show tripcode", "postFormLabelTripcode");
    c.postingDisabledText = currentThread
            ? ts.translate("initBaseBoard", "Posting is disabled for this thread", "postingDisabledText")
            : ts.translate("initBaseBoard", "Posting is disabled for this board", "postingDisabledText");
    c.postingEnabled = postingEnabled;
    c.postingSpeedText = ts.translate("initBaseBoard", "Posting speed:", "postingSpeedText");
    AbstractBoard::PostingSpeed speed = board->postingSpeed();
    double duptime = double(speed.uptimeMsecs) / double(BeQt::Hour);
    qint64 uptime = qint64(duptime);
    std::string shour = ts.translate("initBaseBoard", "post(s) per hour.", "postingSpeed");
    if (!uptime) {
        c.postingSpeed = zeroSpeedString(speed, shour, ts.locale());
    } else if ((speed.postCount / uptime) > 0) {
        c.postingSpeed = speedString(speed, duptime) + " " + shour;
    } else {
        duptime /= 24.0;
        uptime = qint64(duptime);
        std::string sday = ts.translate("initBaseBoard", "post(s) per day.", "postingSpeed");
        if (!uptime) {
            c.postingSpeed = zeroSpeedString(speed, sday, ts.locale());
        } else if ((speed.postCount / uptime) > 0) {
            c.postingSpeed = speedString(speed, duptime) + " " + sday;
        } else {
            duptime /= (365.0 / 12.0);
            uptime = qint64(duptime);
            std::string smonth = ts.translate("initBaseBoard", "post(s) per month.", "postingSpeed");
            if (!uptime) {
                c.postingSpeed = zeroSpeedString(speed, smonth, ts.locale());
            } else if ((speed.postCount / uptime) > 0) {
                c.postingSpeed = speedString(speed, duptime) + " " + smonth;
            } else {
                duptime /= 12.0;
                uptime = qint64(duptime);
                std::string syear = ts.translate("initBaseBoard", "post(s) per year.", "postingSpeed");
                if (!uptime) {
                    c.postingSpeed = zeroSpeedString(speed, syear, ts.locale());
                } else if ((speed.postCount / uptime) > 0) {
                    c.postingSpeed = speedString(speed, duptime) + " " + syear;
                } else {
                    c.postingSpeed = "0 " + syear;
                }
            }
        }
    }
    c.postLimitReachedText = ts.translate("initBaseBoard", "Post limit reached", "postLimitReachedText");
    foreach (QString r, board->postformRules(tq.locale()))
        c.postformRules.push_back(Tools::toStd(r.replace("%currentBoard.name%", board->name())));
    c.quickReplyText = ts.translate("initBaseBoard", "Quick reply", "quickReplyText");
    c.referencedByText = ts.translate("initBaseBoard", "Answers:", "referencedByText");
    c.registeredText = ts.translate("initBaseBoard", "This user is registered", "registeredText");
    c.removeFileText = ts.translate("initBaseBoard", "Remove this file", "removeFileText");
    c.selectFileText = ts.translate("initBaseBoard", "Select file", "selectFileText");
    c.showPostFormText = currentThread ? ts.translate("initBaseBoard", "Answer in this thread", "showPostFormText")
                                       : ts.translate("initBaseBoard", "Create thread", "showPostFormText");
    c.showHidePostText = ts.translate("initBaseBoard", "Hide/show", "showHidePostText");
    c.showWhois = board->showWhois();
    c.supportedFileTypes = Tools::toStd(board->supportedFileTypes());
    c.toBottomText = ts.translate("initBaseBoard", "Scroll to the bottom", "toBottomText");
    c.toThread = ts.translate("initBaseBoard", "Answer", "toThread");
    c.toTopText = ts.translate("initBaseBoard", "Scroll to the top", "toTopText");
    c.unfixThreadText = ts.translate("initBaseBoard", "Unfix thread", "unfixThreadText");
    c.youtubeApiKey = Tools::toStd(s->value("Site/youtube_api_key").toString());
    return true;
}

void redirect(cppcms::application &app, const QString &where)
{
    app.response().set_redirect_header(Tools::toStd(where));
}

void renderBan(cppcms::application &app, const Database::BanInfo &info)
{
    TranslatorQt tq(app.request());
    TranslatorStd ts(app.request());
    Content::Ban c;
    initBase(c, app.request(), tq.translate("renderBan", "Ban", "banBoard"));
    c.banBoard = ("*" != info.boardName) ? Tools::toStd(info.boardName)
                                         : ts.translate("renderBan", "all boards", "pageTitle");
    c.banBoardLabel = ts.translate("renderBan", "Board", "banBoardLabel");
    c.banDateTime = Tools::toStd(ts.locale().toString(Tools::dateTime(info.dateTime, app.request()),
                                                      "dd.MM.yyyy ddd hh:mm:ss"));
    c.banDateTimeLabel = ts.translate("renderBan", "Date", "banDateTimeLabel");
    c.banExpires = info.expires.isValid()
            ? Tools::toStd(ts.locale().toString(Tools::dateTime(info.expires, app.request()),
                                                "dd.MM.yyyy ddd hh:mm:ss"))
            : ts.translate("renderBan", "never", "banExpires");
    c.banExpiresLabel = ts.translate("renderBan", "Expires", "banExpiresLabel");
    if (info.level >= 10)
        c.banLevel = ts.translate("renderBan", "reading and posting are restricted", "pageTitle");
    else if (info.level >= 1)
        c.banLevel = ts.translate("renderBan", "posting is restricted (read-only access)", "pageTitle");
    else
        c.banLevel = ts.translate("renderBan", "no action is restricted", "pageTitle");
    c.banLevelLabel = ts.translate("renderBan", "Restricted actions", "banLevelLabel");
    c.banMessage = ts.translate("renderBan", "You are banned", "banMessage");
    c.banReason = Tools::toStd(info.reason);
    c.banReasonLabel = ts.translate("renderBan", "Reason", "banReasonLabel");
    Tools::render(app, "ban", c);
}

void renderBanAjax(cppcms::application &app, const Database::BanInfo &info)
{
    TranslatorStd ts(app.request());
    cppcms::json::object o;
    o["errorMessage"] = ts.translate("renderBanAjax", "You are banned", "errorMessage");
    std::string desc = ts.translate("renderBanAjax", "Board:", "errorDescription") + " ";
    desc += ("*" != info.boardName) ? Tools::toStd(info.boardName)
                                    : ts.translate("renderBanAjax", "all boards", "errorDescription") + ". ";
    desc += ts.translate("renderBanAjax", "Date:", "errorDescription") + " ";
    desc += Tools::toStd(ts.locale().toString(Tools::dateTime(info.dateTime, app.request()),
                                              "dd.MM.yyyy ddd hh:mm:ss")) + ". ";
    desc += ts.translate("renderBanAjax", "Expires:", "errorDescription") + " ";
    desc += info.expires.isValid() ? Tools::toStd(ts.locale().toString(Tools::dateTime(info.expires, app.request()),
                                                                       "dd.MM.yyyy ddd hh:mm:ss"))
                                   : ts.translate("renderBanAjax", "never", "errorDescription") + ". ";
    desc += ts.translate("renderBanAjax", "Restricted actions:", "errorDescription") + " ";
    if (info.level >= 10)
        desc += ts.translate("renderBanAjax", "reading and posting are restricted", "errorDescription");
    else if (info.level >= 1)
        desc += ts.translate("renderBanAjax", "posting is restricted (read-only access)", "errorDescription");
    else
        desc += ts.translate("renderBanAjax", "no action is restricted", "errorDescription");
    desc += ". " + ts.translate("renderBanAjax", "Reason:", "banReasonLabel") + " " + Tools::toStd(info.reason);
    o["errorDescription"] = desc;
    app.response().out() << cppcms::json::value(o).save();
}

void renderError(cppcms::application &app, const QString &error, const QString &description)
{
    TranslatorQt tq(app.request());
    Content::Error c;
    initBase(c, app.request(), tq.translate("renderError", "Error", "pageTitle"));
    c.errorMessage = !error.isEmpty() ? Tools::toStd(error) : c.pageTitle;
    c.errorDescription = Tools::toStd(description);
    Tools::render(app, "error", c);
}

void renderErrorAjax(cppcms::application &app, const QString &error, const QString &description)
{
    TranslatorStd ts(app.request());
    cppcms::json::object o;
    o["errorMessage"] = !error.isEmpty() ? Tools::toStd(error) : ts.translate("renderError", "Error", "errorMessage");
    o["errorDescription"] = Tools::toStd(description);
    app.response().out() << cppcms::json::value(o).save();
}

void renderIpBan(cppcms::application &app, int level)
{
    TranslatorQt tq(app.request());
    TranslatorStd ts(app.request());
    Content::IpBan c;
    initBase(c, app.request(), tq.translate("renderIpBan", "Ban", "pageTitle"));
    c.banMessage = ts.translate("renderIpBan", "You are banned", "banMessage");
    if (level >= 10) {
        c.banDescription = ts.translate("renderIpBan", "Your IP address is in the ban list. "
                                        "You are not allowed to read or make posts.", "banDescription");
    } else if (level >= 1) {
        c.banDescription = ts.translate("renderIpBan", "Your IP address is in the ban list. "
                                        "You are not allowed to make posts.", "banDescription");
    }
    Tools::render(app, "ip_ban", c);
}

void renderIpBanAjax(cppcms::application &app, int level)
{
    TranslatorStd ts(app.request());
    cppcms::json::object o;
    o["errorMessage"] = ts.translate("renderIpBanAjax", "You are banned", "errorMessage");
    if (level >= 10) {
        o["errorDescription"] = ts.translate("renderIpBanAjax", "Your IP address is in the ban list. "
                                             "You are not allowed to read or make posts.", "errorDescription");
    } else if (level >= 1) {
        o["errorDescription"] = ts.translate("renderIpBanAjax", "Your IP address is in the ban list. "
                                              "You are not allowed to make posts.", "errorDescription");
    }
    app.response().out() << cppcms::json::value(o).save();
}

void renderNotFound(cppcms::application &app)
{
    TranslatorQt tq(app.request());
    TranslatorStd ts(app.request());
    Content::NotFound c;
    initBase(c, app.request(), tq.translate("renderNotFound", "Error 404", "pageTitle"));
    QStringList fns;
    foreach (const QString &path, BCoreApplication::locations(BCoreApplication::DataPath))
        fns << QDir(path + "/static/img/not_found").entryList(QDir::Files);
    if (!fns.isEmpty()) {
        qsrand((uint) QDateTime::currentMSecsSinceEpoch());
        c.imageFileName = Tools::toStd("not_found/" + fns.at(qrand() % fns.size()));
    }
    c.notFoundMessage = ts.translate("renderNotFound", "Page or file not found", "notFoundMessage");
    Tools::render(app, "not_found", c);
}

void renderSuccessfulPostAjax(cppcms::application &app, quint64 postNumber)
{
    cppcms::json::object o;
    o["postNumber"] = postNumber;
    app.response().out() << cppcms::json::value(o).save();
}

void renderSuccessfulThreadAjax(cppcms::application &app, quint64 threadNumber)
{
    cppcms::json::object o;
    o["threadNumber"] = threadNumber;
    app.response().out() << cppcms::json::value(o).save();
}

bool testAddFileParamsAjax(const AbstractBoard *board, cppcms::application &app, const Tools::PostParameters &params,
                           const Tools::FileList &files, QString *error)
{
    TranslatorQt tq(app.request());
    if (!board) {
        QString err = tq.translate("testAddFileParamsAjax", "Internal logic error", "description");
        renderErrorAjax(app, tq.translate("testAddFileParamsAjax", "Internal error", "error"), err);
        return bRet(error, err, false);
    }
    QString err;
    if (!board->testAddFileParams(params, files, tq.locale(), &err)){
        renderErrorAjax(app, tq.translate("testAddFileParamsAjax", "Invalid parameters", "error"), err);
        return false;
    }
    return bRet(error, QString(), true);
}

bool testBan(cppcms::application &app, UserActionType proposedAction, const QString &board)
{
    QString ip = Tools::userIp(app.request());
    int lvl = Tools::ipBanLevel(ip);
    if (lvl >= proposedAction) {
        renderIpBan(app, lvl);
        return false;
    }
    TranslatorQt tq(app.request());
    bool ok = false;
    QString err;
    Database::BanInfo inf = Database::userBanInfo(ip, board, &ok, &err, tq.locale());
    if (!ok) {
        renderError(app, tq.translate("testBan", "Internal error", "error"), err);
        return false;
    }
    if (inf.level >= proposedAction) {
        renderBan(app, inf);
        return false;
    }
    return true;
}

bool testBanAjax(cppcms::application &app, UserActionType proposedAction, const QString &board)
{
    QString ip = Tools::userIp(app.request());
    int lvl = Tools::ipBanLevel(ip);
    if (lvl >= proposedAction) {
        renderIpBanAjax(app, lvl);
        return false;
    }
    TranslatorQt tq(app.request());
    bool ok = false;
    QString err;
    Database::BanInfo inf = Database::userBanInfo(ip, board, &ok, &err, tq.locale());
    if (!ok) {
        renderErrorAjax(app, tq.translate("testBanAjax", "Internal error", "error"), err);
        return false;
    }
    if (inf.level >= proposedAction) {
        renderBanAjax(app, inf);
        return false;
    }
    return true;
}

bool testParamsAjax(const AbstractBoard *board, cppcms::application &app, const Tools::PostParameters &params,
                    const Tools::FileList &files, bool post, QString *error)
{
    TranslatorQt tq(app.request());
    if (!board) {
        QString err = tq.translate("testParamsAjax", "Internal logic error", "description");
        renderErrorAjax(app, tq.translate("testParamsAjax", "Internal error", "error"), err);
        return bRet(error, err, false);
    }
    QString err;
    if (!board->testParams(params, files, post, tq.locale(), &err)){
        renderErrorAjax(app, tq.translate("testParamsAjax", "Invalid parameters", "error"), err);
        return false;
    }
    return bRet(error, QString(), true);
}

bool testRequest(cppcms::application &app, int acceptedTypes, QString *error)
{
    QString r = Tools::fromStd(app.request().request_method());
    bool b = acceptedTypes > 0;
    if ("GET" == r)
        b = b && (acceptedTypes & GetRequest);
    else if ("POST" == r)
        b = b && (acceptedTypes & PostRequest);
    if (b)
        return bRet(error, QString(), true);
    TranslatorQt tq(app.request());
    QString err = tq.translate("testRequest", "Unsupported request type", "error");
    renderError(app, err, tq.translate("testRequest", "This request type is not supported", "error"));
    return bRet(error, err, false);
}

}
