#include "searchroute.h"

#include "controller/controller.h"
#include "controller/search.h"
#include "database.h"
#include "search.h"
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
    if (!Controller::testRequestNonAjax(application, Controller::GetRequest, &err))
        return Tools::log(application, "search", "fail:" + err, logTarget);
    Content::Search c;
    TranslatorQt tq(application.request());
    TranslatorStd ts(application.request());
    Controller::initBase(c, application.request(), tq.translate("SearchRoute", "Search", "pageTitle"));
    c.query = Tools::toStd(query);
    c.queryBoard = Tools::toStd(boardName);
    bool ok = false;
    QString desc;
    Search::Query q = Search::query(query, &ok, &err, tq.locale());
    if (!ok) {
        c.error = true;
        c.errorMessage = ts.translate("SearchRoute", "Query error", "error");
        c.errorDescription = Tools::toStd(err);
        Tools::render(application, "search", c);
        Tools::log(application, "search", "fail:" + err, logTarget);
        return;
    }
    try {
        Transaction t;
        if (!t) {
            c.error = true;
            c.errorMessage = ts.translate("SearchRoute", "Internal error", "error");
            err = tq.translate("SearchRoute", "Internal database error", "description");
            c.errorDescription = Tools::toStd(err);
            Tools::render(application, "search", c);
            Tools::log(application, "search", "fail:" + err, logTarget);
            return;
        }
        if ("*" == boardName)
            boardName.clear();
        QList<Post> list = Database::findPosts(q, boardName, &ok, &err, &desc, tq.locale());
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
            foreach (const QString &phrase, q.requiredPhrases + q.possiblePhrases) {
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
        c.error = true;
        c.errorMessage = ts.translate("SearchRoute", "Internal error", "error");
        err = Tools::fromStd(e.what());
        c.errorDescription = Tools::toStd(err);
        Tools::render(application, "search", c);
        Tools::log(application, "search", "fail:" + err, logTarget);
        return;
    }
    c.error = !ok;
    c.errorMessage = Tools::toStd(err);
    c.errorDescription = Tools::toStd(desc);
    c.resultsMessage = ts.translate("SearchRoute", "Search results", "resultsMessage");
    c.nothingFoundMessage = ts.translate("SearchRoute", "Nothing found", "nothingFoundMessage");
    Tools::render(application, "search", c);
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
