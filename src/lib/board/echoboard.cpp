#include "echoboard.h"

#include "controller/controller.h"
#include "controller/echoboard.h"
#include "controller/echothread.h"
#include "settingslocker.h"
#include "stored/thread.h"
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

echoBoard::echoBoard()
{
    //
}

void echoBoard::beforeStoring(Post *post, const Tools::PostParameters &params, bool thread)
{
    if (!post)
        return;
    if (!thread)
        return;
    post->setUserData(params.value("link"));
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

Content::Post echoBoard::toController(const Post &post, const cppcms::http::request &req, bool *ok,
                                      QString *error) const
{
    bool b = false;
    Content::Post p = AbstractBoard::toController(post, req, &b, error);
    if (!b)
        return bRet(ok, false, p);
    if (p.number == p.threadNumber) {
        QString subj = post.subject();
        QString link = post.userData().toString();
        QString text = BTextTools::toHtml(!subj.isEmpty() ? subj : link);
        p.subject = Tools::toStd("<a href=\"" + link + "\" target=\"_blank\">" + text + "</a>");
        p.subjectIsRaw = true;
    }
    return bRet(ok, true, error, QString(), p);
}

void echoBoard::beforeRenderBoard(const cppcms::http::request &req, Content::Board *c)
{
    Content::echoBoard *cc = dynamic_cast<Content::echoBoard *>(c);
    if (!cc)
        return;
    TranslatorStd ts(req);
    cc->maxLinkLength = 150;
    cc->postFormLabelLink = ts.translate("echoBoard", "Thread link:", "postFormLabelLink");
}

void echoBoard::beforeRenderThread(const cppcms::http::request &/*req*/, Content::Thread *c)
{
    Content::echoThread *cc = dynamic_cast<Content::echoThread *>(c);
    if (!cc)
        return;
    QString link = cc->opPost.userData.toString();
    if (!link.startsWith("https")) {
        QString q = SettingsLocker()->value("Site/ssl_proxy_query").toString();
        if (q.contains("%1"))
            link = q.arg(link);
    }
    cc->threadLink = Tools::toStd(link);
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
