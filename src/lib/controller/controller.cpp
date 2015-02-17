#include "controller.h"

#include "ban.h"
#include "base.h"
#include "baseboard.h"
#include "board/abstractboard.h"
#include "database.h"
#include "error.h"
#include "notfound.h"
#include "settingslocker.h"
#include "stored/banneduser.h"
#include "stored/banneduser-odb.hxx"
#include "tools.h"
#include "translator.h"

#include <BCoreApplication>
#include <BeQt>

#include <QChar>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QLocale>
#include <QMutex>
#include <QSettings>
#include <QString>
#include <QStringList>

#include <cppcms/application.h>
#include <cppcms/http_cookie.h>
#include <cppcms/http_request.h>
#include <cppcms/http_response.h>

#include <odb/connection.hxx>
#include <odb/database.hxx>
#include <odb/query.hxx>
#include <odb/transaction.hxx>

#include <list>

namespace Controller
{

static QMutex localeMutex(QMutex::Recursive);

static Content::Base::Locale toWithLocale(const QLocale &l)
{
    Content::Base::Locale ll;
    QString country = QLocale::countryToString(l.country());
    foreach (int i, bRangeR(country.length() - 1, 1)) {
        const QChar &c = country.at(i);
        if (!c.isUpper())
            continue;
        country.insert(i, ' ');
    }
    ll.country = Tools::toStd(country);
    ll.name = Tools::toStd(l.name());
    ll.language = Tools::toStd(QLocale::languageToString(l.language()));
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
    localeMutex.unlock();
    TranslatorStd ts(req);
    c.boards = AbstractBoard::boardInfos(ts.locale(), false);
    c.currentLocale = toWithLocale(ts.locale());
    c.currentTime = const_cast<cppcms::http::request *>(&req)->cookie_by_name("time").value();
    c.hideSearchFormText = ts.translate("initBaseThread", "Hide search form", "hideSearchFormText");
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
    c.showSearchFormText = ts.translate("initBaseThread", "Search", "showSearchFormText");
    c.sitePathPrefix = Tools::toStd(s->value("Site/path_prefix").toString());
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
    c.action = currentThread ? "create_post" : "create_thread";
    c.ajaxErrorText = ts.translate("initBaseThread", "AJAX request returned status", "ajaxErrorText");
    c.bannedForText = ts.translate("initBaseThread", "User was banned for this post", "bannedForText");
    c.bannerFileName = Tools::toStd(board->bannerFileName());
    c.bumpLimitReachedText = ts.translate("initBaseThread", "Bump limit reached", "bumpLimitReachedText");
    c.captchaEnabled = Tools::captchaEnabled(board->name());
    c.captchaKey = Tools::toStd(SettingsLocker()->value("Site/captcha_public_key").toString());
    c.closedText = ts.translate("initBaseThread", "The thread is closed", "closedText");
    c.closeThreadText = ts.translate("initBaseThread", "Close thread", "closeThreadText");
    c.currentBoard.name = Tools::toStd(board->name());
    c.currentBoard.title = Tools::toStd(board->title(ts.locale()));
    c.currentThread = currentThread;
    c.deletePostText = ts.translate("initBaseThread", "Delete post", "fixedText");
    c.deleteThreadText = ts.translate("initBaseThread", "Delete thread", "fixedText");
    c.enterPasswordText = ts.translate("initBaseThread", "Enter password (if empty, current hashpass will be used):",
                                       "fixedText");
    c.fixedText = ts.translate("initBaseThread", "Fixed", "fixedText");
    c.fixThreadText = ts.translate("initBaseThread", "Fix thread", "fixThreadText");
    c.hidePostFormText = ts.translate("initBaseThread", "Hide post form", "hidePostFormText");
    c.notLoggedInText = ts.translate("initBaseThread", "You are not logged in!", "notLoggedInText");
    c.openThreadText = ts.translate("initBaseThread", "Open thread", "openThreadText");
    c.postFormButtonSubmit = ts.translate("initBaseThread", "Send", "postFormButtonSubmit");
    c.postFormInputFile = ts.translate("initBaseThread", "File:", "postFormInputFile");
    c.postFormInputText = ts.translate("initBaseThread", "Post:", "postFormInputText");
    SettingsLocker s;
    int maxText = s->value("Board/" + board->name() + "/max_text_length",
                           s->value("Board/max_text_length", 15000)).toInt();
    c.postFormInputTextPlaceholder = Tools::toStd(tq.translate("initBaseThread", "Comment. Max length %1",
                                                               "postFormInputTextPlaceholder").arg(maxText));
    c.postFormLabelCaptcha = ts.translate("initBaseThread", "Captcha:", "postFormLabelCaptcha");
    c.postFormLabelEmail = ts.translate("initBaseThread", "E-mail:", "postFormLabelEmail");
    c.postFormLabelName = ts.translate("initBaseThread", "Name:", "postFormLabelName");
    c.postFormLabelPassword = ts.translate("initBaseThread", "Password:", "postFormLabelPassword");
    c.postFormLabelSubject = ts.translate("initBaseThread", "Subject:", "postFormLabelSubject");
    c.postingDisabledText = currentThread
            ? ts.translate("initBaseThread", "Posting is disabled for this thread", "postingDisabledText")
            : ts.translate("initBaseThread", "Posting is disabled for this board", "postingDisabledText");
    c.postingEnabled = postingEnabled;
    c.postLimitReachedText = ts.translate("initBaseThread", "Post limit reached", "postLimitReachedText");
    c.registeredText = ts.translate("initBaseThread", "This user is registered", "registeredText");
    c.showPostFormText = currentThread ? ts.translate("initBaseThread", "Answer in this thread", "showPostFormText")
                                       : ts.translate("initBaseThread", "Create thread", "showPostFormText");
    c.unfixThreadText = ts.translate("initBaseThread", "Unfix thread", "unfixThreadText");
}

void redirect(cppcms::application &app, const QString &where)
{
    app.response().set_redirect_header(Tools::toStd(where));
}

void renderBan(cppcms::application &app, const QString &board, int level, const QDateTime &dateTime,
               const QString &reason, const QDateTime &expires)
{
    TranslatorQt tq(app.request());
    TranslatorStd ts(app.request());
    Content::Ban c;
    initBase(c, app.request(), tq.translate("renderBan", "Ban", "pageTitle"));
    c.banBoard = ("*" != board) ? Tools::toStd(board) : ts.translate("renderBan", "all boards", "pageTitle");
    c.banBoardLabel = ts.translate("renderBan", "Board", "pageTitle");
    c.banDateTime = Tools::toStd(ts.locale().toString(Tools::dateTime(dateTime, app.request()),
                                                      "dd.MM.yyyy ddd hh:mm:ss"));
    c.banDateTimeLabel = ts.translate("renderBan", "Date", "pageTitle");
    c.banExpires = expires.isValid() ? Tools::toStd(ts.locale().toString(Tools::dateTime(expires, app.request()),
                                                                         "dd.MM.yyyy ddd hh:mm:ss"))
                                     : ts.translate("renderBan", "never", "pageTitle");
    c.banExpiresLabel = ts.translate("renderBan", "Expires", "pageTitle");
    if (level >= 10)
        c.banLevel = ts.translate("renderBan", "reading and posting are restricted", "pageTitle");
    else if (level >= 1)
        c.banLevel = ts.translate("renderBan", "posting is restricted (read-only access)", "pageTitle");
    else
        c.banLevel = ts.translate("renderBan", "no action is restricted", "pageTitle");
    c.banLevelLabel = ts.translate("renderBan", "Restricted actions", "pageTitle");
    c.banMessage = ts.translate("renderBan", "You are banned", "pageTitle");
    c.banReason = Tools::toStd(reason);
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

bool testBan(cppcms::application &app, UserActionType proposedAction, const QString &board)
{
    QString ip = Tools::userIp(app.request());
    TranslatorQt tq(app.request());
    try {
        odb::database *db = Database::createConnection();
        if (!db) {
            renderError(app, tq.translate("testBan", "Internal error", "error"),
                        tq.translate("testBan", "Internal database error", "description"));
            return false;
        }
        odb::transaction transaction(db->begin());
        odb::result<BannedUser> r(db->query<BannedUser>(odb::query<BannedUser>::board == "*"
                                                        && odb::query<BannedUser>::ip == ip));
        QString banBoard;
        QDateTime banDateTime;
        QString banReason;
        QDateTime banExpires;
        int banLevel = 0;
        for (odb::result<BannedUser>::iterator i = r.begin(); i != r.end(); ++i) {
            QDateTime expires = i->expirationDateTime().toUTC();
            if (!expires.isValid() || expires > QDateTime::currentDateTimeUtc()) {
                banLevel = i->level();
                banBoard = i->board();
                banDateTime = i->dateTime();
                banReason = i->reason();
                banExpires = i->expirationDateTime();
                break;
            }
        }
        if (banLevel <= 0) {
            odb::result<BannedUser> rr(db->query<BannedUser>(odb::query<BannedUser>::board == board
                                                            && odb::query<BannedUser>::ip == ip));
            for (odb::result<BannedUser>::iterator i = rr.begin(); i != rr.end(); ++i) {
                QDateTime expires = i->expirationDateTime().toUTC();
                if (!expires.isValid() || expires > QDateTime::currentDateTimeUtc()) {
                    banLevel = i->level();
                    banBoard = i->board();
                    banDateTime = i->dateTime().toUTC();
                    banReason = i->reason();
                    banExpires = i->expirationDateTime();
                    break;
                }
            }
        }
        if (banLevel <= 0)
            return true;
        if (banLevel < proposedAction)
            return true;
        renderBan(app, banBoard, banLevel, banDateTime, banReason, banExpires);
        transaction.commit();
        return false;
    }  catch (const odb::exception &e) {
        renderError(app, tq.translate("testBan", "Internal error", "error"), Tools::fromStd(e.what()));
        return false;
    }
}

bool testParams(cppcms::application &app, const Tools::PostParameters &params, const Tools::FileList &files,
                const QString &boardName)
{
    SettingsLocker s;
    TranslatorQt tq(app.request());
    int maxEmail = s->value("Board/" + boardName + "/max_email_length",
                            s->value("Board/max_email_length", 150)).toInt();
    int maxName = s->value("Board/" + boardName + "/max_name_length",
                           s->value("Board/max_name_length", 50)).toInt();
    int maxSubject = s->value("Board/" + boardName + "/max_subject_length",
                              s->value("Board/max_subject_length", 150)).toInt();
    int maxText = s->value("Board/" + boardName + "/max_text_length",
                           s->value("Board/max_text_length", 15000)).toInt();
    int maxPassword = s->value("Board/" + boardName + "/max_password_length",
                                s->value("Board/max_password_length", 150)).toInt();
    int maxFileCount = s->value("Board/" + boardName + "/max_file_count",
                                s->value("Board/max_file_count", 1)).toInt();
    int maxFileSize = s->value("Board/" + boardName + "/max_file_size",
                               s->value("Board/max_file_size", 10 * BeQt::Megabyte)).toInt();
    if (params.value("email").length() > maxEmail) {
        renderError(app, tq.translate("testParams", "Invalid parameters", "error"),
                    tq.translate("testParams", "E-mail is too long", "description"));
        Tools::log(app, "E-mail is too long");
        return false;
    } else if (params.value("name").length() > maxName) {
        renderError(app, tq.translate("testParams", "Invalid parameters", "error"),
                    tq.translate("testParams", "Name is too long", "description"));
        Tools::log(app, "Name is too long");
        return false;
    } else if (params.value("subject").length() > maxSubject) {
        renderError(app, tq.translate("testParams", "Invalid parameters", "error"),
                    tq.translate("testParams", "Subject is too long", "description"));
        Tools::log(app, "Subject is too long");
        return false;
    } else if (params.value("text").length() > maxText) {
        renderError(app, tq.translate("testParams", "Invalid parameters", "error"),
                    tq.translate("testParams", "Comment is too long", "description"));
        Tools::log(app, "Comment is too long");
        return false;
    } else if (params.value("password").length() > maxPassword) {
        renderError(app, tq.translate("testParams", "Invalid parameters", "error"),
                    tq.translate("testParams", "Password is too long", "description"));
        Tools::log(app, "Password is too long");
        return false;
    } else if (files.size() > maxFileCount) {
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
