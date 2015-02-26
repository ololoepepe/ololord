#include "echoboard.h"

#include "controller/board.h"
#include "controller/controller.h"
#include "controller/thread.h"
#include "tools.h"
#include "translator.h"

#include <QLocale>
#include <QRegExp>
#include <QString>
#include <QStringList>

#include <list>
#include <string>

echoBoard::echoBoard()
{
    //
}


QString echoBoard::name() const
{
    return "echo";
}

QString echoBoard::title(const QLocale &l) const
{
    TranslatorQt tq(l);
    return tq.translate("echoBoard", "Boardsphere echo", "board title");
}

void echoBoard::beforeRenderBoard(const cppcms::http::request &req, Content::Board *c)
{
    if (!c)
        return;
    TranslatorStd ts(req);
    c->postFormLabelSubject = ts.translate("echoBoard", "Thread link:", "postFormLabelSubject");
    for (std::list<Content::Board::Thread>::iterator i = c->threads.begin(); i != c->threads.end(); ++i) {
        i->opPost.subject = Tools::toStd(Controller::toHtml(Tools::fromStd(i->opPost.subject)));
        i->opPost.subjectAlwaysRaw = true;
    }
}

void echoBoard::beforeRenderThread(const cppcms::http::request &/*req*/, Content::Thread *c)
{
    if (!c)
        return;
    c->opPost.subject = Tools::toStd(Controller::toHtml(Tools::fromStd(c->opPost.subject)));
    c->opPost.subjectAlwaysRaw = true;
}

Content::Thread *echoBoard::createThreadController(const cppcms::http::request &/*req*/, QString &viewName)
{
    viewName = "echo_thread";
    return new Content::Thread;
}

bool echoBoard::testParam(ParamType t, const QString &param, bool post, const QLocale &l, QString *error) const
{
    if (!AbstractBoard::testParam(t, param, post, l, error))
        return false;
    if (SubjectParam != t || post)
        return true;
    TranslatorQt tq(l);
    if (param.isEmpty())
        return bRet(error, tq.translate("echoBoard", "Link to external thread is empty", "description"), false);
    foreach (const QString &s, Tools::acceptedExternalBoards()) {
        if (QRegExp(s).exactMatch(param))
            return true;
    }
    return bRet(error, tq.translate("echoBoard", "This board/thread may not be accepted", "description"), false);
}
