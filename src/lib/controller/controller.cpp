#include "controller.h"

#include "ban.h"
#include "base.h"
#include "baseboard.h"
#include "board/abstractboard.h"
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

#include <list>

namespace Controller
{

static QMutex localeMutex(QMutex::Recursive);

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
        styles.insert("", BTranslation::translate("initBase", "Photon", "style name"));
    }
    localeMutex.unlock();
    TranslatorStd ts(req);
    c.boards = AbstractBoard::boardInfos(ts.locale(), false);
    c.cancelButtonText = ts.translate("initBase", "Cancel", "cancelButtonText");
    c.confirmButtonText = ts.translate("initBase", "Confirm", "confirmButtonText");
    c.currentLocale = toWithLocale(ts.locale());
    c.currentTime = const_cast<cppcms::http::request *>(&req)->cookie_by_name("time").value();
    c.hideSearchFormText = ts.translate("initBase", "Hide search form", "hideSearchFormText");
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
    c.pageTitle = Tools::toStd(pageTitle);
    SettingsLocker s;
    c.searchApiKey = Tools::toStd(s->value("Site/search_api_key").toString());
    c.showPasswordText = ts.translate("initBase", "Show password", "showPasswordText");
    c.showSearchFormText = ts.translate("initBase", "Search", "showSearchFormText");
    c.showTripcodeText = ts.translate("initBase", "I'm an attention whore!", "showTripcodeText");
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
}

void initBaseBoard(Content::BaseBoard &c, const cppcms::http::request &req, const AbstractBoard *board,
                   bool postingEnabled, const QString &pageTitle, quint64 currentThread)
{
    if (!board)
        return;
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
        AbstractBoard *b = AbstractBoard::board(s);
        inf.title = Tools::toStd(b ? b->title(tq.locale()) : tq.translate("initBaseBoard", "All boards", "boardName"));
        c.availableBoards.push_back(inf);
    }
    c.action = currentThread ? "create_post" : "create_thread";
    c.ajaxErrorText = ts.translate("initBaseBoard", "AJAX request returned status", "ajaxErrorText");
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
    c.captchaEnabled = Tools::captchaEnabled(board->name()) && !board->captchaQuota(ip);
    c.captchaKey = Tools::toStd(SettingsLocker()->value("Site/captcha_public_key").toString());
    c.captchaQuota = board->captchaQuota(ip);
    c.captchaQuotaText = ts.translate("initBaseBoard", "Posts without captcha left:", "captchaQuotaText");
    c.closedText = ts.translate("initBaseBoard", "The thread is closed", "closedText");
    c.closeThreadText = ts.translate("initBaseBoard", "Close thread", "closeThreadText");
    c.complainText = ts.translate("initBaseBoard", "Complain", "complainText");
    c.complainMessage = ts.translate("initBaseBoard", "Go complain to your mum, you whiner!", "complainMessage");
    c.currentBoard.name = Tools::toStd(board->name());
    c.currentBoard.title = Tools::toStd(board->title(ts.locale()));
    c.currentThread = currentThread;
    c.deletePostText = ts.translate("initBaseBoard", "Delete post", "fixedText");
    c.deleteThreadText = ts.translate("initBaseBoard", "Delete thread", "fixedText");
    c.downloadThreadText = ts.translate("initBaseBoard", "Download all thread files as a .zip archive",
                                        "downloadThreadText");
    c.editPostText = ts.translate("initBaseBoard", "Edit post", "editPostText");
    c.enterPasswordText = ts.translate("initBaseBoard", "If password is empty, current hashpass will be used",
                                       "enterPasswordText");
    c.enterPasswordTitle = ts.translate("initBaseBoard", "Enter password", "enterPasswordTitle");
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
    c.megabytesText = ts.translate("initBaseBoard", "MB", "megabytesText");
    c.moder = Database::registeredUserLevel(req) / 10;
    if (c.moder > 0) {
        QStringList boards = Database::registeredUserBoards(req);
        if (!boards.contains("*") && !boards.contains(board->name()))
            c.moder = 0;
    }
    c.noCaptchaText = ts.translate("initBaseBoard", "You don't have to enter captcha", "noCaptchaText");
    c.notLoggedInText = ts.translate("initBaseBoard", "You are not logged in!", "notLoggedInText");
    c.openThreadText = ts.translate("initBaseBoard", "Open thread", "openThreadText");
    c.postFormButtonSubmit = ts.translate("initBaseBoard", "Send", "postFormButtonSubmit");
    c.postFormInputFile = ts.translate("initBaseBoard", "File(s):", "postFormInputFile");
    SettingsLocker s;
    int maxText = s->value("Board/" + board->name() + "/max_text_length",
                           s->value("Board/max_text_length", 15000)).toInt();
    c.postFormTextPlaceholder = Tools::toStd(tq.translate("initBaseBoard", "Comment. Max length %1",
                                                           "postFormTextPlaceholder").arg(maxText));
    c.postFormLabelCaptcha = ts.translate("initBaseBoard", "Captcha:", "postFormLabelCaptcha");
    c.postFormLabelEmail = ts.translate("initBaseBoard", "E-mail:", "postFormLabelEmail");
    c.postFormLabelName = ts.translate("initBaseBoard", "Name:", "postFormLabelName");
    c.postFormLabelPassword = ts.translate("initBaseBoard", "Password:", "postFormLabelPassword");
    c.postFormLabelRaw = ts.translate("initBaseBoard", "Raw HTML:", "postFormLabelRaw");
    c.postFormLabelSubject = ts.translate("initBaseBoard", "Subject:", "postFormLabelSubject");
    c.postFormLabelText = ts.translate("initBaseBoard", "Post:", "postFormLabelText");
    c.postingDisabledText = currentThread
            ? ts.translate("initBaseBoard", "Posting is disabled for this thread", "postingDisabledText")
            : ts.translate("initBaseBoard", "Posting is disabled for this board", "postingDisabledText");
    c.postingEnabled = postingEnabled;
    c.postLimitReachedText = ts.translate("initBaseBoard", "Post limit reached", "postLimitReachedText");
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
    initBase(c, app.request(), tq.translate("renderBan", "Ban", "pageTitle"));
    c.banBoard = ("*" != info.boardName) ? Tools::toStd(info.boardName)
                                         : ts.translate("renderBan", "all boards", "pageTitle");
    c.banBoardLabel = ts.translate("renderBan", "Board", "pageTitle");
    c.banDateTime = Tools::toStd(ts.locale().toString(Tools::dateTime(info.dateTime, app.request()),
                                                      "dd.MM.yyyy ddd hh:mm:ss"));
    c.banDateTimeLabel = ts.translate("renderBan", "Date", "pageTitle");
    c.banExpires = info.expires.isValid()
            ? Tools::toStd(ts.locale().toString(Tools::dateTime(info.expires, app.request()),
                                                "dd.MM.yyyy ddd hh:mm:ss"))
            : ts.translate("renderBan", "never", "pageTitle");
    c.banExpiresLabel = ts.translate("renderBan", "Expires", "pageTitle");
    if (info.level >= 10)
        c.banLevel = ts.translate("renderBan", "reading and posting are restricted", "pageTitle");
    else if (info.level >= 1)
        c.banLevel = ts.translate("renderBan", "posting is restricted (read-only access)", "pageTitle");
    else
        c.banLevel = ts.translate("renderBan", "no action is restricted", "pageTitle");
    c.banLevelLabel = ts.translate("renderBan", "Restricted actions", "pageTitle");
    c.banMessage = ts.translate("renderBan", "You are banned", "pageTitle");
    c.banReason = Tools::toStd(info.reason);
    c.banReasonLabel = ts.translate("renderBan", "Reason", "pageTitle");
    app.render("ban", c);
    Tools::log(app, "Banned");
}

void renderError(cppcms::application &app, const QString &error, const QString &description)
{
    TranslatorQt tq(app.request());
    TranslatorStd ts(app.request());
    Content::Error c;
    initBase(c, app.request(), tq.translate("renderError", "Error", "pageTitle"));
    c.errorMessage = !error.isEmpty() ? Tools::toStd(error) : c.pageTitle;
    c.errorDescription = Tools::toStd(description);
    app.render("error", c);
    Tools::log(app, error + (!description.isEmpty() ? (": " + description) : QString()));
}

void renderIpBan(cppcms::application &app, int level)
{
    TranslatorQt tq(app.request());
    TranslatorStd ts(app.request());
    Content::IpBan c;
    initBase(c, app.request(), tq.translate("renderIpBan", "Ban", "pageTitle"));
    c.banMessage = ts.translate("renderIpBan", "You are banned", "pageTitle");
    if (level >= 10) {
        c.banDescription = ts.translate("renderIpBan", "Your IP address is in the ban list. "
                                        "You are not allowed to read or make posts.", "pageTitle");
    } else if (level >= 1) {
        c.banDescription = ts.translate("renderIpBan", "Your IP address is in the ban list. "
                                        "You are not allowed to make posts.", "pageTitle");
    }
    app.render("ip_ban", c);
    Tools::log(app, "Banned (ban list)");
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
    //c.imageTitle = ts.translate("renderNotFound", "Page or file not found", "imageTitle");
    c.notFoundMessage = ts.translate("renderNotFound", "Page or file not found", "notFoundMessage");
    app.render("not_found", c);
    Tools::log(app, "Page or file not found");
}

void renderSuccessfulPost(cppcms::application &app, quint64 postNumber, const QSet<quint64> &referencedPosts)
{
    app.response().out() << "<html><head><meta http-equiv=\"Content-Type\" content=\"text/html;charset=utf-8\">"
                         << "<title></title></head><body><input id=\"postNumber\" type=\"hidden\" "
                         << "value=\"" << postNumber << "\" /> ";
    QList<quint64> list = referencedPosts.toList();
    qSort(list);
    foreach (quint64 pn, list)
        app.response().out() << "<input name=\"referencedPost\" type=\"hidden\" value=\"" << pn << "\" />";
    app.response().out() << " </body></html>";
}

void renderSuccessfulThread(cppcms::application &app, quint64 threadNumber)
{
    app.response().out() << "<html><head><meta http-equiv=\"Content-Type\" content=\"text/html;charset=utf-8\">"
                         << "<title></title></head><body><input id=\"threadNumber\" type=\"hidden\" "
                         << "value=\"" << threadNumber << "\" /></body></html>";
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

bool testParams(const AbstractBoard *board, cppcms::application &app, const Tools::PostParameters &params,
                const Tools::FileList &files, bool post)
{
    TranslatorQt tq(app.request());
    if (!board) {
        renderError(app, tq.translate("testParams", "Internal error", "error"),
                    tq.translate("testParams", "Internal logic error", "description"));
        return false;
    }
    QString boardName = board->name();
    int maxFileSize = Tools::maxInfo(Tools::MaxFileSize, boardName);
    QString err;
    if (!board->testParams(params, post, tq.locale(), &err)){
        renderError(app, tq.translate("testParams", "Invalid parameters", "error"), err);
        Tools::log(app, "Invalid field");
        return false;
    } else if (files.size() > int(Tools::maxInfo(Tools::MaxFileCount, boardName))) {
        renderError(app, tq.translate("testParams", "Invalid parameters", "error"),
                    tq.translate("testParams", "Too many files", "description"));
        Tools::log(app, "File is too big");
        return false;
    } else {
        foreach (const Tools::File &f, files) {
            if (f.data.size() > maxFileSize) {
                renderError(app, tq.translate("testParams", "Invalid parameters", "error"),
                            tq.translate("testParams", "File is too big", "description"));
                Tools::log(app, "File is too big");
                return false;
            }
            if (!board->isFileTypeSupported(f.data)) {
                renderError(app, tq.translate("testParams", "Invalid parameters", "error"),
                            tq.translate("testParams", "File type is not supported", "description"));
                Tools::log(app, "File type is not supported");
                return false;
            }
        }
    }
    return true;
}

bool testRequest(cppcms::application &app, int acceptedTypes)
{
    QString r = Tools::fromStd(app.request().request_method());
    bool b = acceptedTypes > 0;
    if ("GET" == r)
        b = b && (acceptedTypes & GetRequest);
    else if ("POST" == r)
        b = b && (acceptedTypes & PostRequest);
    if (b)
        return true;
    TranslatorQt tq(app.request());
    renderError(app, tq.translate("testRequest", "Unsupported request type", "error"),
                tq.translate("testRequest", "This request type is not supported", "error"));
    return false;
}

}
