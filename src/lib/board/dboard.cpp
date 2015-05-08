#include "dboard.h"

#include "controller/baseboard.h"
#include "stored/thread.h"
#include "tools.h"
#include "translator.h"

#include <BTextTools>

#include <QDebug>
#include <QLocale>
#include <QString>
#include <QVariant>

#include <cppcms/http_request.h>

#include <string>

dBoard::dBoard()
{
    //
}

bool dBoard::beforeStoringNewPost(const cppcms::http::request &req, Post *post,
                                  const Tools::PostParameters &/*params*/, bool /*thread*/, QString *error,
                                  QString *description)
{
    TranslatorQt tq(req);
    if (!post) {
        return bRet(error, tq.translate("dBoard", "Internal error", "error"), description,
                    tq.translate("dBoard", "Internal logic error", "description"), false);
    }
    QString userAgent = Tools::fromStd(const_cast<cppcms::http::request *>(&req)->http_user_agent());
    post->setUserData(userAgent);
    return bRet(error, QString(), description, QString(), true);
}

QString dBoard::name() const
{
    return "d";
}

QString dBoard::title(const QLocale &l) const
{
    return TranslatorQt(l).translate("dBoard", "Board /d/iscussion", "board title");
}

Content::Post dBoard::toController(const Post &post, const cppcms::http::request &req, bool *ok,
                                      QString *error) const
{
    bool b = false;
    Content::Post p = AbstractBoard::toController(post, req, &b, error);
    if (!b)
        return bRet(ok, false, p);
    QString userAgent = post.userData().toString();
    if (!userAgent.isEmpty()) {
        QString text = Tools::fromStd(p.text) + "<font face=\"monospace\">"
                + BTextTools::toHtml("\n\n" + QString().fill('-', 50) + "\n" + userAgent) + "</font>";
        p.text = Tools::toStd(text);
    }
    return bRet(ok, true, error, QString(), p);
}
