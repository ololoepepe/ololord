#include "echoboard.h"

#include "controller/controller.h"
#include "controller/echoboard.h"
#include "controller/echothread.h"
#include "settingslocker.h"
#include "tools.h"
#include "translator.h"

#include <BTextTools>

#include <QDebug>
#include <QLocale>
#include <QRegExp>
#include <QSettings>
#include <QString>
#include <QStringList>
#include <QVariant>

#include <list>
#include <string>

static QString processPost(Content::BaseBoard::Post &p, QString *subject = 0)
{
    QString s = Tools::fromStd(p.subject);
    QString subj = BTextTools::removeTrailingSpaces(s.mid(0, 1000));
    QString link = s.mid(1000);
    p.subject = Tools::toStd("<a href=\"" + link + "\">" + BTextTools::toHtml(!subj.isEmpty() ? subj : link) + "</a>");
    p.subjectIsRaw = true;
    return bRet(subject, subj, link);
}

echoBoard::echoBoard()
{
    //
}


QString echoBoard::name() const
{
    return "echo";
}

bool echoBoard::testParams(const Tools::PostParameters &params, bool post, const QLocale &l, QString *error) const
{
    if (!AbstractBoard::testParams(params, post, l, error))
        return false;
    if (post)
        return true;
    TranslatorQt tq(l);
    QString link = params.value("link");
    if (link.isEmpty())
        return bRet(error, tq.translate("echoBoard", "Thread link is empty", "description"), false);
    else if (link.length() > 150)
        return bRet(error, tq.translate("echoBoard", "Thread link is too long", "description"), false);
    foreach (const QString &s, Tools::acceptedExternalBoards()) {
        if (QRegExp(s).exactMatch(link))
            return true;
    }
    return bRet(error, tq.translate("echoBoard", "This board/thread may not be accepted", "description"), false);
}

QString echoBoard::title(const QLocale &l) const
{
    TranslatorQt tq(l);
    return tq.translate("echoBoard", "Boardsphere echo", "board title");
}

void echoBoard::beforeRenderBoard(const cppcms::http::request &req, Content::Board *c)
{
    Content::echoBoard *cc = dynamic_cast<Content::echoBoard *>(c);
    if (!cc)
        return;
    TranslatorStd ts(req);
    cc->maxLinkLength = 150;
    cc->postFormLabelLink = ts.translate("echoBoard", "Thread link:", "postFormLabelLink");
    for (std::list<Content::Board::Thread>::iterator i = c->threads.begin(); i != c->threads.end(); ++i)
        processPost(i->opPost);
}

void echoBoard::beforeRenderThread(const cppcms::http::request &/*req*/, Content::Thread *c)
{
    Content::echoThread *cc = dynamic_cast<Content::echoThread *>(c);
    if (!cc)
        return;
    QString subj;
    QString link = processPost(cc->opPost, &subj);
    QString q = SettingsLocker()->value("Site/ssl_proxy_query").toString();
    if (!link.startsWith("https") && q.contains("%1"))
        link = q.arg(link);
    cc->threadLink = Tools::toStd(link);
    cc->pageTitle = Tools::toStd(subj);
}

void echoBoard::beforeStoring(Tools::PostParameters &params, bool post)
{
    if (post)
        return;
    QString subject = params.value("subject");
    QString link = params.value("link");
    params["subject"] = BTextTools::appendTrailingSpaces(subject, 1000) + link;
}

Content::Board *echoBoard::createBoardController(const cppcms::http::request &/*req*/, QString &viewName)
{
    viewName = "echo_board";
    return new Content::echoBoard;
}

Content::Thread *echoBoard::createThreadController(const cppcms::http::request &/*req*/, QString &viewName)
{
    viewName = "echo_thread";
    return new Content::echoThread;
}
