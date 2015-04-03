#include "searchroute.h"

#include "controller/controller.h"
#include "controller/search.h"
#include "database.h"
#include "stored/thread.h"
#include "stored/thread-odb.hxx"
#include "tools.h"
#include "transaction.h"
#include "translator.h"

#include <BTextTools>

#include <QDebug>
#include <QList>
#include <QRegExp>
#include <QString>
#include <QStringList>

#include <cppcms/application.h>
#include <cppcms/http_request.h>

SearchRoute::SearchRoute(cppcms::application &app) :
    AbstractRoute(app)
{
    //
}

void SearchRoute::handle()
{
    Tools::GetParameters params = Tools::getParameters(application.request());
    QString query = params.value("query");
    QString boardName = params.value("board");
    if (boardName.isEmpty())
        boardName = "*";
    QString logTarget = boardName + "/" + query;
    Tools::log(application, "search", "begin", logTarget);
    QString err;
    if (!Controller::testRequest(application, Controller::GetRequest, &err))
        return Tools::log(application, "search", "fail:" + err, logTarget);
    Content::Search c;
    TranslatorQt tq(application.request());
    TranslatorStd ts(application.request());
    Controller::initBase(c, application.request(), tq.translate("SearchRoute", "Search", "pageTitle"));
    c.query = Tools::toStd(query);
    c.queryBoard = Tools::toStd(boardName);
    Database::FindPostsParameters p(tq.locale());
    bool ok = false;
    QStringList phrases = BTextTools::splitCommand(query, &ok);
    if (!ok) {
        c.error = false;
        c.errorMessage = ts.translate("SearchRoute", "Invalid query", "error");
        err = tq.translate("SearchRoute", "Invalid search query", "description");
        c.errorDescription = Tools::toStd(err);
        application.render("search", c);
        Tools::log(application, "search", "fail:" + err, logTarget);
        return;
    }
    foreach (const QString &phrase, phrases) {
        if (phrase.startsWith('+'))
            p.requiredPhrases << phrase.mid(1);
        else if (phrase.startsWith('-'))
            p.excludedPhrases << phrase.mid(1);
        else
            p.possiblePhrases << phrase;
    }
    if ("*" != boardName)
        p.boardNames << boardName;
    QString desc;
    p.ok = &ok;
    p.error = &err;
    p.description = &desc;
    try {
        Transaction t;
        if (!t) {
            c.error = false;
            c.errorMessage = ts.translate("SearchRoute", "Internal error", "error");
            err = tq.translate("SearchRoute", "Internal database error", "description");
            c.errorDescription = Tools::toStd(err);
            application.render("search", c);
            Tools::log(application, "search", "fail:" + err, logTarget);
            return;
        }
        QList<Post> list = Database::findPosts(p);
        foreach (Post post, list) {
            Content::Search::SearchResult r;
            r.boardName = Tools::toStd(post.board());
            r.postNumber = post.number();
            QSharedPointer<Thread> thread = post.thread().load();
            r.threadNumber = thread->number();
            QString txt = post.rawText();
            txt.replace(QRegExp("\r*\n+"), " ");
            if (txt.length() > 300)
                txt = txt.left(297) + "...";
            QString subj = !post.subject().isEmpty() ? post.subject() : txt;
            if (subj.length() > 100)
                subj = subj.left(97) + "...";
            r.subject = Tools::toStd(subj);
            txt = Controller::toHtml(txt);
            foreach (const QString &phrase, p.requiredPhrases + p.possiblePhrases) {
                int ind = txt.indexOf(phrase, Qt::CaseInsensitive);
                while (ind >= 0) {
                    QString nphrase = "<b><font color=\"red\">" + phrase + "</font></b>";
                    txt.replace(ind, phrase.length(), nphrase);
                    ind = txt.indexOf(phrase, ind + nphrase.length(), Qt::CaseInsensitive);
                }
            }
            r.text = Tools::toStd(txt);
            c.searchResults.push_back(r);
        }
        t.commit();
    } catch (const odb::exception &e) {
        c.error = false;
        c.errorMessage = ts.translate("SearchRoute", "Internal error", "error");
        err = Tools::fromStd(e.what());
        c.errorDescription = Tools::toStd(err);
        application.render("search", c);
        Tools::log(application, "search", "fail:" + err, logTarget);
        return;
    }
    c.error = !ok;
    c.errorMessage = Tools::toStd(err);
    c.errorDescription = Tools::toStd(desc);
    c.resultsMessage = ts.translate("SearchRoute", "Search results", "resultsMessage");
    c.nothingFoundMessage = ts.translate("SearchRoute", "Nothing found", "nothingFoundMessage");
    application.render("search", c);
    Tools::log(application, "search", "success", logTarget);
}

unsigned int SearchRoute::handlerArgumentCount() const
{
    return 0;
}

std::string SearchRoute::key() const
{
    return "search";
}

int SearchRoute::priority() const
{
    return 5;
}

std::string SearchRoute::regex() const
{
    return "/search";
}

std::string SearchRoute::url() const
{
    return "/search";
}
