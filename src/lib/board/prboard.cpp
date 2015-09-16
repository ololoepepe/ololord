#include "prboard.h"

#include "controller/board.h"
#include "controller/prboard.h"
#include "controller/prthread.h"
#include "controller/thread.h"
#include "settingslocker.h"
#include "tools.h"
#include "translator.h"

#include <QDebug>
#include <QLocale>
#include <QSettings>
#include <QString>
#include <QVariant>

#include <string>

prBoard::prBoard()
{
    //
}

AbstractBoard::MarkupElements prBoard::markupElements() const
{
    return MarkupElements(BoldMarkupElement | ItalicsMarkupElement | StrikedOutMarkupElement | UnderlinedMarkupElement
            | SpoilerMarkupElement | QuotationMarkupElement | CodeMarkupElement | SubscriptMarkupElement
            | SuperscriptMarkupElement | UrlMarkupElement);
}

QString prBoard::name() const
{
    return "pr";
}

QString prBoard::supportedCaptchaEngines() const
{
    return "codecha";
}

QString prBoard::title(const QLocale &l) const
{
    TranslatorQt tq(l);
    return tq.translate("prBoard", "/pr/ogramming", "board title");
}

void prBoard::beforeRenderBoard(const cppcms::http::request &/*req*/, Content::Board *c)
{
    Content::prBoard *cc = dynamic_cast<Content::prBoard *>(c);
    if (!cc)
        return;
    cc->codechaPublicKey = Tools::toStd(SettingsLocker()->value("Site/codecha_public_key").toString());
}

void prBoard::beforeRenderThread(const cppcms::http::request &/*req*/, Content::Thread *c)
{
    Content::prThread *cc = dynamic_cast<Content::prThread *>(c);
    if (!cc)
        return;
    cc->codechaPublicKey = Tools::toStd(SettingsLocker()->value("Site/codecha_public_key").toString());
}

Content::Board *prBoard::createBoardController(const cppcms::http::request &/*req*/, QString &viewName)
{
    viewName = "pr_board";
    return new Content::prBoard;
}

Content::Thread *prBoard::createThreadController(const cppcms::http::request &/*req*/, QString &viewName)
{
    viewName = "pr_thread";
    return new Content::prThread;
}
