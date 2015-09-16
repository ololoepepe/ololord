#include "markuproute.h"

#include "controller.h"
#include "controller/markup.h"
#include "tools.h"
#include "translator.h"

#include <BeQtGlobal>

#include <QDebug>
#include <QString>
#include <QStringList>

#include <cppcms/application.h>
#include <cppcms/http_request.h>

MarkupRoute::MarkupRoute(cppcms::application &app) :
    AbstractRoute(app)
{
    //
}

void MarkupRoute::handle()
{
    DDOS_A(11)
    Tools::log(application, "markup", "begin");
    QString err;
    if (!Controller::testRequestNonAjax(application, Controller::GetRequest, &err)) {
        Tools::log(application, "markup", "fail:" + err);
        DDOS_POST_A
        return;
    }
    Content::Markup c;
    TranslatorQt tq(application.request());
    TranslatorStd ts(application.request());
    Controller::initBase(c, application.request(), tq.translate("MarkupRoute", "Markup", "pageTitle"));
    c.basicMarkup = ts.translate("MarkupRoute", "Basic markup", "basicMarkup");
    c.boldText = ts.translate("MarkupRoute", "bold text", "boldText");
    c.cspoilerText = ts.translate("MarkupRoute", "collapsible spoiler text", "cspoilerText");
    c.cspoilerTitle = ts.translate("MarkupRoute", "collapsible spoiler title", "cspoilerTitle");
    c.codeMarkup = ts.translate("MarkupRoute", "Code markup", "codeMarkup");
    c.combinedText = ts.translate("MarkupRoute", "combined text", "combinedText");
    c.doubleHyphen = ts.translate("MarkupRoute", "double hyphen", "doubleHyphen");
    c.emDash = ts.translate("MarkupRoute", "em dash", "emDash");
    c.enDash = ts.translate("MarkupRoute", "en dash", "enDash");
    c.italics = ts.translate("MarkupRoute", "italics", "italics");
    c.linkMarkup = ts.translate("MarkupRoute", "Link markup", "linkMarkup");
    c.listItem1 = ts.translate("MarkupRoute", "first list item", "listItem1");
    c.listItem2 = ts.translate("MarkupRoute", "second list item", "listItem2");
    c.listItem3 = ts.translate("MarkupRoute", "third list item", "listItem3");
    c.listMarkup = ts.translate("MarkupRoute", "List markup", "listMarkup");
    c.postBoardLinkDescription = ts.translate("MarkupRoute", "Link to a post on another board",
                                              "postBoardLinkDescription");
    c.postLinkDescription = ts.translate("MarkupRoute", "Link to a post on the same board", "postLinkDescription");
    c.preformattedText = ts.translate("MarkupRoute", "Preformatted\ntext", "preformattedText");
    c.quadripleHyphen = ts.translate("MarkupRoute", "quadriple hyphen", "quadripleHyphen");
    c.quotation = ts.translate("MarkupRoute", "quotation", "quotation");
    c.replacementMarkup = ts.translate("MarkupRoute", "Auto replacement", "replacementMarkup");
    c.monospace = ts.translate("MarkupRoute", "monospace font", "monospace");
    c.monospaceEscaped = ts.translate("MarkupRoute", "monospace \\`` font with escaped \\`` characters",
                                      "monospaceEscaped");
    c.nomarkup = ts.translate("MarkupRoute", "no markup", "nomarkup");
    c.nomarkupEscaped = ts.translate("MarkupRoute", "no markup \\'' with escaped \\'' characters", "nomarkupEscaped");
    c.spoiler = ts.translate("MarkupRoute", "spoiler", "spoiler");
    c.strikedoutText = ts.translate("MarkupRoute", "striked out text", "strikedoutText");
    c.subscript = ts.translate("MarkupRoute", "subscript", "subscriptText");
    c.superscript = ts.translate("MarkupRoute", "superscript", "superscriptText");
    c.tooltip = ts.translate("MarkupRoute", "tooltip", "tooltip");
    c.tooltipText = ts.translate("MarkupRoute", "text with tooltip", "tooltipText");
    foreach (int i, bRangeD(0, c.strikedoutText.size() - 1)) {
        Q_UNUSED(i)
        c.strikedoutTextWakaba.append("^H");
    }
    c.strikedoutWord1 = ts.translate("MarkupRoute", "striked out", "strikedoutWord1");
    c.strikedoutWord2 = ts.translate("MarkupRoute", "word", "strikedoutWord2");
    c.underlinedText = ts.translate("MarkupRoute", "underlined text", "underlinedText");
    Tools::render(application, "markup", c);
    Tools::log(application, "markup", "success");
    DDOS_POST_A
}

unsigned int MarkupRoute::handlerArgumentCount() const
{
    return 0;
}

std::string MarkupRoute::key() const
{
    return "markup";
}

int MarkupRoute::priority() const
{
    return 0;
}

std::string MarkupRoute::regex() const
{
    return "/markup";
}

std::string MarkupRoute::url() const
{
    return "/markup";
}
