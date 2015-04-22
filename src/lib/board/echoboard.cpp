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

#include <cppcms/http_request.h>
#include <cppcms/json.h>

#include <list>
#include <string>

echoBoard::echoBoard()
{
    //
}

bool echoBoard::beforeStoringEditedPost(const cppcms::http::request &req, cppcms::json::value &userData, Post &p,
                                        Thread &thread, QString *error)
{
    try {
        if (userData.is_null() || userData.is_undefined())
            return bRet(error, QString(), true);
        TranslatorQt tq(req);
        if (p.number() != thread.number())
            return bRet(error, tq.translate("echoBoard", "Attempt to edit link of non-OP post", "error"), false);
        QString link = Tools::fromStd(userData.str());
        bool ok = false;
        foreach (const QString &s, Tools::acceptedExternalBoards()) {
            if (QRegExp(s).exactMatch(link)) {
                ok = true;
                break;
            }
        }
        if (!ok)
            return bRet(error, tq.translate("echoBoard", "This board/thread may not be accepted", "error"), false);
        if (!link.startsWith("http"))
            link.prepend("http://");
        p.setUserData(link);
        return bRet(error, QString(), true);
    } catch (const cppcms::json::bad_value_cast &e) {
        return bRet(error, Tools::fromStd(e.what()), false);
    }
}

bool echoBoard::beforeStoringNewPost(const cppcms::http::request &req, Post *post, const Tools::PostParameters &params,
                                     bool thread, QString *error, QString *description)
{
    TranslatorQt tq(req);
    if (!post) {
        return bRet(error, tq.translate("echoBoard", "Internal error", "error"), description,
                    tq.translate("echoBoard", "Internal logic error", "description"), false);
    }
    if (!thread)
        return bRet(error, QString(), description, QString(), true);
    QString link = params.value("link");
    if (!link.startsWith("http"))
        link.prepend("http://");
    post->setUserData(link);
    return bRet(error, QString(), description, QString(), true);
}

QString echoBoard::name() const
{
    return "echo";
}

bool echoBoard::testParams(const Tools::PostParameters &params, const Tools::FileList &files, bool post,
                           const QLocale &l, QString *error) const
{
    if (!AbstractBoard::testParams(params, files, post, l, error))
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
    return TranslatorQt(l).translate("echoBoard", "Boardsphere echo", "board title");
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

cppcms::json::object echoBoard::toJson(const Content::Post &post, const cppcms::http::request &req) const
{
    cppcms::json::object o = AbstractBoard::toJson(post, req);
    o["link"] = Tools::toStd(post.userData.toString());
    return o;
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

void echoBoard::beforeRenderThread(const cppcms::http::request &req, Content::Thread *c)
{
    Content::echoThread *cc = dynamic_cast<Content::echoThread *>(c);
    if (!cc)
        return;
    TranslatorStd ts(req);
    cc->maxLinkLength = 150;
    cc->postFormLabelLink = ts.translate("echoBoard", "Thread link:", "postFormLabelLink");
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
