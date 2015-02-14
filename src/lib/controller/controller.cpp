#include "controller.h"

#include "ban.h"
#include "board/abstractboard.h"
#include "database.h"
#include "error.h"
#include "notfound.h"
#include "settingslocker.h"
#include "stored/banneduser.h"
#include "stored/banneduser-odb.hxx"
#include "tools.h"
#include "translator.h"
#include "withbanner.h"
#include "withbase.h"
#include "withnavbar.h"
#include "withpostform.h"
#include "withposts.h"
#include "withsettings.h"

#include <BCoreApplication>

#include <QChar>
#include <QDateTime>
#include <QDir>
#include <QLocale>
#include <QMutex>
#include <QSettings>
#include <QString>
#include <QStringList>

#include <cppcms/application.h>
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

static WithSettings::Locale toWithLocale(const QLocale &l)
{
    WithSettings::Locale ll;
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

void initWithBanner(WithBanner *w, const QLocale &/*l*/, const AbstractBoard *board)
{
    if (!w || !board)
        return;
    w->bannerFileName = Tools::toStd(board->bannerFileName());
}

void initWithBase(WithBase *w, const QLocale &/*l*/)
{
    if (!w)
        return;
    w->sitePathPrefix = Tools::toStd(SettingsLocker()->value("Site/path_prefix").toString());
}

void initWithNavbar(WithNavbar *w, const QLocale &l)
{
    if (!w)
        return;
    TranslatorStd ts(l);
    w->boards = AbstractBoard::boardInfos(ts.locale(), false);
    w->toHomePageText = ts.translate("initWithNavbar", "Home", "toHomePageText");
}

void initWithPostForm(WithPostForm *w, const QLocale &l, const AbstractBoard *board)
{
    if (!w || !board)
        return;
    TranslatorStd ts(l);
    w->captchaEnabled = Tools::captchaEnabled(board->name());
    w->captchaKey = Tools::toStd(SettingsLocker()->value("Site/captcha_public_key").toString());
    w->currentBoard.name = Tools::toStd(board->name());
    w->currentBoard.title = board->title(l);
    w->postFormButtonSubmit = ts.translate("initWithPostForm", "Send", "postFormButtonSubmit");
    w->postFormInputFile = ts.translate("initWithPostForm", "File:", "postFormInputFile");
    w->postFormInputText = ts.translate("initWithPostForm", "Post:", "postFormInputText");
    w->postFormInputTextPlaceholder = ts.translate("initWithPostForm", "Comment. Max length 15000",
                                                   "postFormInputTextPlaceholder");
    w->postFormLabelCaptcha = ts.translate("initWithPostForm", "Captcha:", "postFormLabelCaptcha");
    w->postFormLabelEmail = ts.translate("initWithPostForm", "E-mail:", "postFormLabelEmail");
    w->postFormLabelName = ts.translate("initWithPostForm", "Name:", "postFormLabelName");
    w->postFormLabelPassword = ts.translate("initWithPostForm", "Password:", "postFormLabelPassword");
    w->postFormLabelSubject = ts.translate("initWithPostForm", "Subject:", "postFormLabelSubject");
}

void initWithPosts(WithPosts *w, const QLocale &l)
{
    if (!w)
        return;
    TranslatorStd ts(l);
    w->bannedForText = ts.translate("initWithPosts", "User was banned for this post", "bannedForText");
    w->bumpLimitReachedText = ts.translate("initWithPosts", "Bump limit reached", "bumpLimitReachedText");
    w->closedText = ts.translate("initWithPosts", "The thread is closed", "closedText");
    w->fixedText = ts.translate("initWithPosts", "Fixed", "fixedText");
}

void initWithSettings(WithSettings *w, const QLocale &l)
{
    typedef std::list<WithSettings::Locale> LocaleList;
    localeMutex.lock();
    init_once(LocaleList, locales, LocaleList()) {
        foreach (const QString &path, BCoreApplication::locations(BCoreApplication::TranslationsPath)) {
            foreach (const QString &fn, QDir(path).entryList(QStringList() << "ololord_*.qm", QDir::Files)) {
                QString ln = fn.mid(8);
                ln.remove(ln.length() - 3, 3);
                QLocale ll(ln);
                if (QLocale::c() == ll)
                    continue;
                WithSettings::Locale lll = toWithLocale(ll);
                if (std::find(locales.begin(), locales.end(), lll) != locales.end())
                    continue;
                locales.push_back(lll);
            }
        }
        locales.push_back(toWithLocale(QLocale("en_US")));
    }
    localeMutex.unlock();
    if (!w)
        return;
    w->currentLocale = toWithLocale(l);
    w->localeLabelText = "Language:";
    w->locales = locales;
}

void redirect(cppcms::application &app, const QString &where)
{
    app.response().set_redirect_header(Tools::toStd(where));
}

void renderBan(cppcms::application &app, const QString &board, int level, const QDateTime &dateTime,
               const QString &reason, const QDateTime &expires)
{
    TranslatorStd ts(app.request());
    Content::Ban c;
    initWithBase(&c, ts.locale());
    initWithNavbar(&c, ts.locale());
    initWithSettings(&c, ts.locale());
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
    c.pageTitle = ts.translate("renderBan", "Ban", "pageTitle");
    app.render("ban", c);
    Tools::log(app, "Banned");
}

void renderError(cppcms::application &app, const QString &error, const QString &description)
{
    TranslatorStd ts(app.request());
    Content::Error c;
    initWithBase(&c, ts.locale());
    initWithNavbar(&c, ts.locale());
    initWithSettings(&c, ts.locale());
    c.pageTitle = ts.translate("renderError", "Error", "pageTitle");
    c.errorMessage = !error.isEmpty() ? Tools::toStd(error) : c.pageTitle;
    c.errorDescription = Tools::toStd(description);
    app.render("error", c);
    Tools::log(app, error + (!description.isEmpty() ? (": " + description) : QString()));
}

void renderNotFound(cppcms::application &app)
{
    TranslatorStd ts(app.request());
    Content::NotFound c;
    initWithBase(&c, ts.locale());
    initWithNavbar(&c, ts.locale());
    initWithSettings(&c, ts.locale());
    QStringList fns;
    foreach (const QString &path, BCoreApplication::locations(BCoreApplication::DataPath))
        fns << QDir(path + "/static/img/not_found").entryList(QDir::Files);
    if (!fns.isEmpty()) {
        qsrand((uint) QDateTime::currentMSecsSinceEpoch());
        c.imageFileName = Tools::toStd("not_found/" + fns.at(qrand() % fns.size()));
    }
    //c.imageTitle = ts.translate("renderNotFound", "Page or file not found", "imageTitle");
    c.notFoundMessage = ts.translate("renderNotFound", "Page or file not found", "notFoundMessage");
    c.pageTitle = ts.translate("renderNotFound", "Error 404", "pageTitle");
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

bool testParams(cppcms::application &app, const Tools::PostParameters &params)
{
    TranslatorQt tq(app.request());
    if (params.value("email").length() > 150) {
        renderError(app, tq.translate("testParams", "Invalid parameters", "error"),
                    tq.translate("testParams", "E-mail is too long", "description"));
        Tools::log(app, "E-mail is too long");
        return false;
    } else if (params.value("name").length() > 50) {
        renderError(app, tq.translate("testParams", "Invalid parameters", "error"),
                    tq.translate("testParams", "Name is too long", "description"));
        Tools::log(app, "Name is too long");
        return false;
    } else if (params.value("subject").length() > 150) {
        renderError(app, tq.translate("testParams", "Invalid parameters", "error"),
                    tq.translate("testParams", "Subject is too long", "description"));
        Tools::log(app, "Subject is too long");
        return false;
    } else if (params.value("text").length() > 15000) {
        renderError(app, tq.translate("testParams", "Invalid parameters", "error"),
                    tq.translate("testParams", "Comment is too long", "description"));
        Tools::log(app, "Comment is too long");
        return false;
    } else if (params.value("password").length() > 150) {
        renderError(app, tq.translate("testParams", "Invalid parameters", "error"),
                    tq.translate("testParams", "Password is too long", "description"));
        Tools::log(app, "Password is too long");
        return false;
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
