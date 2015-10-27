#include "markup.h"

#include "cache.h"
#include "controller/baseboard.h"
#include "database.h"
#include "settingslocker.h"
#include "tools.h"
#include "translator.h"

#include <BDirTools>
#include <BeQt>
#include <BTextTools>

#include <QByteArray>
#include <QChar>
#include <QCryptographicHash>
#include <QDateTime>
#include <QDebug>
#include <QFileInfo>
#include <QImage>
#include <QList>
#include <QLocale>
#include <QMap>
#include <QPair>
#include <QRegExp>
#include <QSettings>
#include <QSharedPointer>
#include <QStack>
#include <QString>
#include <QStringList>
#include <QVariant>

#include <cppcms/http_request.h>

#include <srchilite/ioexception.h>
#include <srchilite/langmap.h>
#include <srchilite/parserexception.h>
#include <srchilite/sourcehighlight.h>

#include <exception>
#include <list>
#include <sstream>
#include <string>

namespace Markup
{

class ProcessingInfo
{
public:
    enum SkipType
    {
        NoSkip = 0,
        HtmlSkip,
        CodeSkip
    };
private:
    struct SkipInfo
    {
        int from;
        int length;
        SkipType type;
    };
public:
    const QString BoardName;
    const quint64 DeletedPost;
    Database::RefMap * const ReferencedPosts;
private:
    QList<SkipInfo> skipList;
    QString &mtext;
public:
    explicit ProcessingInfo(QString &txt, const QString &boardName, Database::RefMap *referencedPosts,
                            quint64 deletedPost);
public:
    int find(const QRegExp &rx, int from = 0, bool escapable = false) const;
    bool in(int start, int length, SkipType type = CodeSkip) const;
    void insert(int from, const QString &txt, SkipType type = HtmlSkip);
    void replace(int from, int length, const QString &txt, int correction, SkipType type = HtmlSkip);
    const QString &text() const;
    QString toHtml() const;
};

struct ListInfo
{
    int count;
    int length;
    int pos;
    QString type;
};

static bool isEscaped(const QString &s, int pos);
static QString withoutEscaped(const QString &text);

ProcessingInfo::ProcessingInfo(QString &txt, const QString &boardName, Database::RefMap *referencedPosts,
                               quint64 deletedPost) :
    BoardName(boardName), DeletedPost(deletedPost), ReferencedPosts(referencedPosts), mtext(txt)
{
    //
}

int ProcessingInfo::find(const QRegExp &rx, int from, bool escapable) const
{
    int ind = rx.indexIn(mtext, from);
    while (ind >= 0) {
        bool in = false;
        foreach (int i, bRangeD(0, skipList.length() - 1)) {
            const SkipInfo &inf = skipList.at(i);
            if (ind >= inf.from && ind < (inf.from + inf.length)) {
                ind = rx.indexIn(mtext, inf.from + inf.length);
                in = true;
                break;
            }
        }
        if (!in) {
            if (escapable && isEscaped(mtext, ind))
                ind = rx.indexIn(mtext, ind + 1);
            else
                return ind;
        }
    }
    return -1;
}

bool ProcessingInfo::in(int start, int length, SkipType type) const
{
    if (start < 0 || length <= 0 || (start + length) > mtext.length() || NoSkip == type)
        return false;
    foreach (int i, bRangeD(0, skipList.length() - 1)) {
        const SkipInfo &inf = skipList.at(i);
        if (inf.type != type)
            continue;
        int x = start;
        while (x < start + length) {
            if (x >= inf.from && x <= (inf.from + inf.length))
                return true;
            ++x;
        }
    }
    return false;
}

void ProcessingInfo::insert(int from, const QString &txt, SkipType type)
{
    if (from < 0 || txt.length() <= 0 || from > mtext.length())
        return;
    SkipInfo info;
    info.from = from;
    info.length = txt.length();
    info.type = type;
    bool found = false;
    foreach (int i, bRangeR(skipList.length() - 1, 0)) {
        SkipInfo &inf = skipList[i];
        if (from > inf.from) {
            if (NoSkip != type)
                skipList.insert(i + 1, info);
            found = true;
            break;
        }
        inf.from += txt.length();
    }
    if (!found && NoSkip != type)
        skipList.prepend(info);
    mtext.insert(from, txt);
}

void ProcessingInfo::replace(int from, int length, const QString &txt, int correction, SkipType type)
{
    if (from < 0 || length <= 0 || txt.isEmpty() || (length + from) > mtext.length())
        return;
    SkipInfo info;
    info.from = from;
    info.length = txt.length();
    info.type = type;
    bool found = false;
    foreach (int i, bRangeR(skipList.length() - 1, 0)) {
        SkipInfo &inf = skipList[i];
        if (from >= inf.from) {
            if (NoSkip != type)
                skipList.insert(i + 1, info);
            found = true;
            break;
        }
        if (inf.from < (from + length))
            inf.from -= correction;
        else
            inf.from += (txt.length() - length);
    }
    if (!found && NoSkip != type)
        skipList.prepend(info);
    mtext.replace(from, length, txt);
}

const QString &ProcessingInfo::text() const
{
    return mtext;
}

QString ProcessingInfo::toHtml() const
{
    QString s;
    int last = 0;
    foreach (int i, bRangeD(0, skipList.length() - 1)) {
        const SkipInfo &inf = skipList.at(i);
        s += BTextTools::toHtml(withoutEscaped(mtext.mid(last, inf.from - last)), false);
        s += mtext.mid(inf.from, inf.length);
        last = inf.from + inf.length;
    }
    s += BTextTools::toHtml(mtext.mid(last), false);
    s.replace(QRegExp("</li>(\\s|&nbsp;|<br />)+<li"), "</li><li");
    s.replace(QRegExp("</li>(\\s|&nbsp;|<br />)+</ul"), "</li></ul");
    s.replace(QRegExp("</li>(\\s|&nbsp;|<br />)+</ol"), "</li></ol");
    s.replace(QRegExp("<ol>(\\s|&nbsp;|<br />)+<li"), "<ol><li");
    QRegExp rx("<ul type\\=\"(disc|circle|square)\">(\\s|&nbsp;|<br />)+<li");
    int ind = rx.indexIn(s);
    while (ind >= 0) {
        QString ns = "<ul type=\"" + rx.cap(1) + "\"><li";
        s.replace(ind, rx.matchedLength(), ns);
        ind = rx.indexIn(s, ns.length());
    }
    return s;
}

bool isEscaped(const QString &s, int pos)
{
    if (pos <= 0 || pos >= s.length())
        return false;
    int n = 0;
    int i = pos - 1;
    while (i >= 0 && s.at(i) == '\\') {
        ++n;
        --i;
    }
    return (n % 2);
}

QString withoutEscaped(const QString &text)
{
    QString ntext = text;
    int ind = ntext.lastIndexOf(QRegExp("``|''"));
    while (ind >= 0) {
        if (isEscaped(ntext, ind)) {
            ntext.remove(ind - 1, 1);
            ind = ntext.lastIndexOf(QRegExp("``|''"), ind - ntext.length() - 3);
            continue;
        }
        ind = ntext.lastIndexOf(QRegExp("``|''"), ind - ntext.length() - 2);
    }
    return ntext;
}

typedef bool (*CheckFunction)(ProcessingInfo &info, const QRegExp &rxOp, const QRegExp &rxCl, int inds, int inde);
typedef QString (*ConversionFunction)(ProcessingInfo &info, const QString &text, const QRegExp &rxOp,
                                      const QRegExp &rxCl, QString &op, QString &cl, ProcessingInfo::SkipType &type);

static int getIndE(ProcessingInfo &info, const QRegExp &rxOp, const QRegExp &rxCl, int inds, bool nestable,
                   bool escapable, bool &nested)
{
    nested = false;
    if (!nestable)
        return (inds >= 0) ? info.find(rxCl, inds + rxOp.matchedLength(), escapable) : -1;
    QRegExp rxOpT(rxOp);
    if (inds >= 0) {
        int indst = info.find(rxOpT, inds + rxOp.matchedLength(), escapable);
        int indet = info.find(rxCl, inds + rxOp.matchedLength(), escapable);
        int depth = 1;
        while (indst >= 0 || indet >= 0) {
            int tmp = (indst >= 0 && indst < indet) ? indst : indet;
            int offs = (indst >= 0 && indst < indet) ? rxOpT.matchedLength() : rxCl.matchedLength();
            depth += (tmp == indst) ? 1 : -1;
            if (depth > 1)
                nested = true;
            if (!depth)
                return tmp;
            indst = info.find(rxOpT, tmp + offs, escapable);
            indet = info.find(rxCl, tmp + offs, escapable);
        }
    }
    return -1;
}

static void process(ProcessingInfo &info, ConversionFunction conversionFunction, const QRegExp &rxOp,
                    const QRegExp &rxCl, bool nestable = false, bool escapable = false,
                    CheckFunction checkFunction = 0)
{
    bool nested = false;
    int inds = info.find(rxOp, 0, escapable);
    int inde = !rxCl.isEmpty() ? getIndE(info, rxOp, rxCl, inds, nestable, escapable, nested) : -1;
    bool rerun = false;
    while (inds >= 0 && (rxCl.isEmpty() || inde > inds)) {
        if (checkFunction && !checkFunction(info, rxOp, rxCl, inds, inde)) {
            if (!rxCl.isEmpty())
                inds = info.find(rxOp, inde + rxCl.matchedLength(), escapable);
            else
                inds = info.find(rxOp, inds + rxOp.matchedLength(), escapable);
            inde = !rxCl.isEmpty() ? getIndE(info, rxOp, rxCl, inds, nestable, escapable, nested) : -1;
            continue;
        }
        QString op;
        QString cl;
        ProcessingInfo::SkipType type = ProcessingInfo::NoSkip;
        QString txt = info.text().mid(inds + rxOp.matchedLength(), inde - inds - rxOp.matchedLength());
        txt = conversionFunction(info, txt, rxOp, rxCl, op, cl, type);
        if (!txt.isEmpty()) {
            if (!cl.isEmpty())
                info.insert(!rxCl.isEmpty() ? (inde + rxCl.matchedLength()) : inds + rxOp.matchedLength(), cl);
            if (!rxCl.isEmpty())
                info.replace(inds, inde - inds + rxCl.matchedLength(), txt, rxOp.matchedLength(), type);
            else
                info.replace(inds, rxOp.matchedLength(), txt, rxOp.matchedLength(), type);
            if (!op.isEmpty())
                info.insert(inds, op);
            inds = info.find(rxOp, inds + txt.length() + op.length() + cl.length(), escapable);
        } else {
            if (!rxCl.isEmpty())
                inds = info.find(rxOp, inde + rxCl.matchedLength(), escapable);
            else
                inds = info.find(rxOp, inds + rxOp.matchedLength(), escapable);
        }
        if (nestable && nested)
            rerun = true;
        inde = !rxCl.isEmpty() ? getIndE(info, rxOp, rxCl, inds, nestable, escapable, nested) : -1;
    }
    if (rerun)
        process(info, conversionFunction, rxOp, rxCl, nestable, escapable, checkFunction);
}

static void process(ProcessingInfo &info, ConversionFunction conversionFunction, const QRegExp &rxOp,
                    bool nestable = false, bool escapable = false, CheckFunction checkFunction = 0)
{
    QRegExp rxCl(rxOp);
    return process(info, conversionFunction, rxOp, rxCl, nestable, escapable, checkFunction);
}

static void processStrikedOutShitty(ProcessingInfo &info)
{
    QRegExp rx("(\\^H)+");
    int ind = info.find(rx);
    while (ind >= 0) {
        int s = ind - (rx.matchedLength() / 2);
        if (s < 0) {
            ind = info.find(rx, ind + rx.matchedLength());
            continue;
        }
        info.replace(ind, rx.matchedLength(), "</s>", 0);
        info.insert(s, "<s>");
        ind = info.find(rx, ind + 7);
    }
}

static void processStrikedOutShittyWord(ProcessingInfo &info)
{
    QRegExp rx("(\\^W)+");
    int ind = info.find(rx);
    const QString &txt = info.text();
    while (ind >= 0) {
        int count = rx.matchedLength() / 2;
        int pcount = count;
        int s = ind - 1;
        while (count > 0) {
            while (s >= 0 && txt.at(s).isSpace())
                --s;
            while (s >= 0 && !txt.at(s).isSpace())
                --s;
            --count;
        }
        info.replace(ind, rx.matchedLength(), "</s>", 0);
        info.insert(s + 1, "<s>");
        ind = info.find(rx, ind + (7 * pcount));
    }
}

static bool checkLangsMatch(ProcessingInfo &, const QRegExp &rxOp, const QRegExp &rxCl, int, int)
{
    return !rxOp.cap(1).isEmpty() && rxOp.cap(1) == rxCl.cap(1);
}

static bool checkExternalLink(ProcessingInfo &, const QRegExp &rxOp, const QRegExp &, int, int)
{
    return rxOp.cap(2).count('.') == 3 || Tools::externalLinkRootZoneExists(rxOp.cap(4));
}

static bool checkNotInterrupted(ProcessingInfo &info, const QRegExp &, const QRegExp &, int inds, int inde)
{
    if (info.in(inds, inde - inds))
        return false;
    return (0 == inds || "\n" == info.text().mid(inds - 1, 1) || (info.in(inds - 6, 6, ProcessingInfo::HtmlSkip)
                                                                  && info.text().mid(inds - 6, 6) == "<br />"));
}

static QString convertMonospace(ProcessingInfo &, const QString &text, const QRegExp &, const QRegExp &, QString &op,
                                QString &cl, ProcessingInfo::SkipType &type)
{
    op = "<font face=\"monospace\">";
    cl = "</font>";
    type = ProcessingInfo::CodeSkip;
    return BTextTools::toHtml(withoutEscaped(text), false);
}

static QString convertNomarkup(ProcessingInfo &, const QString &text, const QRegExp &, const QRegExp &, QString &,
                               QString &, ProcessingInfo::SkipType &type)
{
    type = ProcessingInfo::CodeSkip;
    return BTextTools::toHtml(withoutEscaped(text), false);
}

static QString convertPre(ProcessingInfo &, const QString &text, const QRegExp &, const QRegExp &, QString &op,
                          QString &cl, ProcessingInfo::SkipType &type)
{
    op = "<pre>";
    cl = "</pre>";
    type = ProcessingInfo::CodeSkip;
    return withoutEscaped(text);
}

static QString convertCode(ProcessingInfo &, const QString &text, const QRegExp &rxOp, const QRegExp &, QString &op,
                           QString &cl, ProcessingInfo::SkipType &type)
{
    init_once(QString, srchighlightPath, QString())
        srchighlightPath = BDirTools::findResource("srchilite", BDirTools::AllResources);
    if (srchighlightPath.isEmpty())
        return "";
    op = "<div class=\"codeBlock\">";
    cl = "</div>";
    type = ProcessingInfo::CodeSkip;
    QString lang = rxOp.cap(1);
    lang.replace("++", "pp");
    if (lang.isEmpty())
        lang = "nohilite";
    std::istringstream in(Tools::toStd(text));
    std::ostringstream out;
    try {
        srchilite::SourceHighlight sourceHighlight("html.outlang");
        sourceHighlight.setDataDir(Tools::toStd(srchighlightPath));
        sourceHighlight.highlight(in, out, Tools::toStd(lang + ".lang"));
    } catch (const srchilite::ParserException &e) {
        Tools::log("Markup::convertCode", e);
        return "";
    } catch (const srchilite::IOException &e) {
        Tools::log("Markup::convertCode", e);
        return "";
    } catch (const std::exception &e) {
        Tools::log("Markup::convertCode", e);
        return "";
    }
    return Tools::fromStd(out.str());
}

static QString convertExternalLink(ProcessingInfo &, const QString &, const QRegExp &rxOp, const QRegExp &,
                                   QString &, QString &, ProcessingInfo::SkipType &type)
{
    type = ProcessingInfo::HtmlSkip;
    QString href = rxOp.cap();
    if (!href.startsWith("http") && !href.startsWith("ftp"))
        href.prepend("http://");
    return "<a href=\"" + href + "\">" + BTextTools::toHtml(rxOp.cap()) + "</a>";
}

static QString convertProtocol(ProcessingInfo &, const QString &, const QRegExp &rxOp, const QRegExp &, QString &,
                               QString &, ProcessingInfo::SkipType &type)
{
    type = ProcessingInfo::HtmlSkip;
    return "<a href=\"" + rxOp.cap() + "\">" + BTextTools::toHtml(rxOp.cap(2)) + "</a>";
}

static QString convertTooltipShitty(ProcessingInfo &, const QString &, const QRegExp &rxOp, const QRegExp &,
                                    QString &op, QString &cl, ProcessingInfo::SkipType &type)
{
    QString tooltip = rxOp.cap(2);
    type = ProcessingInfo::NoSkip;
    op = "<span class=\"tooltip\" title=\"" + tooltip + "\">";
    cl = "</span>";
    return rxOp.cap(1);
}

static QString convertPostLink(ProcessingInfo &info, const QString &, const QRegExp &rxOp, const QRegExp &,
                               QString &, QString &, ProcessingInfo::SkipType &type)
{
    type = ProcessingInfo::HtmlSkip;
    QString prefix = SettingsLocker()->value("Site/path_prefix").toString();
    QString boardName = (rxOp.captureCount() > 1) ? rxOp.cap(1) : info.BoardName;
    QString postNumber = rxOp.cap((rxOp.captureCount() > 1) ? 2 : 1);
    quint64 pn = postNumber.toULongLong();
    quint64 tn = 0;
    if (pn && (pn != info.DeletedPost) && Database::postExists(boardName, pn, &tn)) {
        if (info.ReferencedPosts)
            info.ReferencedPosts->insert(Database::RefKey(boardName, pn), tn);
        QString threadNumber = QString::number(tn);
        QString href = "href=\"/" + prefix + boardName + "/thread/" + threadNumber + ".html#" + postNumber + "\"";
        return "<a " + href + ">" + rxOp.cap().replace(">", "&gt;") + "</a>";
    } else {
        return rxOp.cap().replace(">", "&gt;");
    }
}

static QString convertMarkup(ProcessingInfo &, const QString &text, const QRegExp &rxOp, const QRegExp &, QString &op,
                             QString &cl, ProcessingInfo::SkipType &type)
{
    typedef QPair<QString, QString> WrapperPair;
    typedef QMap<QString, WrapperPair> WrapperMap;
    init_once(WrapperMap, map, WrapperMap()) {
        map.insert("---", qMakePair(QString("<s>"), QString("</s>")));
        map.insert("***", qMakePair(QString("<u>"), QString("</u>")));
        map.insert("**", qMakePair(QString("<strong>"), QString("</strong>")));
        map.insert("*", qMakePair(QString("<em>"), QString("</em>")));
        map.insert("___", qMakePair(QString("<u>"), QString("</u>")));
        map.insert("__", qMakePair(QString("<strong>"), QString("</strong>")));
        map.insert("_", qMakePair(QString("<em>"), QString("</em>")));
        map.insert("///", qMakePair(QString("<em>"), QString("</em>")));
        map.insert("%%", qMakePair(QString("<span class=\"spoiler\">"), QString("</span>")));
        map.insert("[b]", qMakePair(QString("<strong>"), QString("</strong>")));
        map.insert("[i]", qMakePair(QString("<em>"), QString("</em>")));
        map.insert("[s]", qMakePair(QString("<s>"), QString("</s>")));
        map.insert("[u]", qMakePair(QString("<u>"), QString("</u>")));
        map.insert("[sub]", qMakePair(QString("<sub>"), QString("</sub>")));
        map.insert("[sup]", qMakePair(QString("<sup>"), QString("</sup>")));
        map.insert("[spoiler]", qMakePair(QString("<span class=\"spoiler\">"), QString("</span>")));
    }
    type = ProcessingInfo::NoSkip;
    if ("----" == rxOp.cap())
        return "\u2014";
    else if ("--" == rxOp.cap())
        return "\u2013";
    WrapperPair p = map.value(rxOp.cap());
    if (p.first.isEmpty())
        return "";
    op = p.first;
    cl = p.second;
    return text;
}

static QString convertUrl(ProcessingInfo &, const QString &text, const QRegExp &, const QRegExp &, QString &,
                          QString &, ProcessingInfo::SkipType &type)
{
    if (text.isEmpty())
        return "";
    type = ProcessingInfo::HtmlSkip;
    QString href = text;
    if (!href.startsWith("http") && !href.startsWith("ftp"))
        href.prepend("http://");
    return "<a href=\"" + href + "\">" + BTextTools::toHtml(text) + "</a>";
}

static QString convertCSpoiler(ProcessingInfo &, const QString &text, const QRegExp &rxOp, const QRegExp &,
                               QString &op, QString &cl, ProcessingInfo::SkipType &type)
{
    QString title = rxOp.cap(1);
    if (title.isEmpty())
        title = "Spoiler";
    type = ProcessingInfo::NoSkip;
    op = "<span class=\"cspoiler\"><span class=\"cspoilerTitle\" title=\"Spoiler\" "
         "onclick=\"lord.expandCollapseSpoiler(this);\">" + title
          + "</span><span class=\"cspoilerBody\" style=\"display: none;\">";
    cl = "</span></span>";
    return text;
}

static QString convertTooltip(ProcessingInfo &, const QString &text, const QRegExp &rxOp, const QRegExp &, QString &op,
                              QString &cl, ProcessingInfo::SkipType &type)
{
    QString tooltip = rxOp.cap(1);
    type = ProcessingInfo::NoSkip;
    op = "<span class=\"tooltip\" title=\"" + tooltip + "\">";
    cl = "</span>";
    return text;
}

static QString convertUnorderedList(ProcessingInfo &, const QString &text, const QRegExp &rxOp, const QRegExp &,
                                    QString &op, QString &cl, ProcessingInfo::SkipType &type)
{
    typedef QMap<QString, QString> StringMap;
    init_once(StringMap, map, StringMap()) {
        map.insert("d", "disc");
        map.insert("c", "circle");
        map.insert("s", "square");
    }
    QString t = rxOp.cap(1);
    if (t.length() == 1)
        t = map.value(t);
    if (t.isEmpty())
        return "";
    type = ProcessingInfo::NoSkip;
    op = "<ul type=\"" + t + "\">";
    cl = "</ul>";
    return text;
}

static QString convertOrderedList(ProcessingInfo &, const QString &text, const QRegExp &, const QRegExp &,
                                  QString &op, QString &cl, ProcessingInfo::SkipType &type)
{
    type = ProcessingInfo::NoSkip;
    op = "<ol>";
    cl = "</ol>";
    return text;
}

static QString convertListItem(ProcessingInfo &, const QString &text, const QRegExp &rxOp, const QRegExp &,
                               QString &op, QString &cl, ProcessingInfo::SkipType &type)
{
    type = ProcessingInfo::NoSkip;
    op = "<li";
    if (!rxOp.cap(2).isEmpty())
        op += " value=\"" + rxOp.cap(2) + "\"";
    op += ">";
    cl = "</li>";
    return text;
}

static QString convertCitation(ProcessingInfo &, const QString &text, const QRegExp &rxOp, const QRegExp &rxCl,
                               QString &op, QString &cl, ProcessingInfo::SkipType &type)
{
    type = ProcessingInfo::NoSkip;
    if (rxOp.cap(1) == "\n")
        op = "<br />";
    op += "<span class=\"quotation\">&gt;";
    cl = "</span>";
    if (rxCl.cap() == "\n")
        cl += "<br />";
    return text;
}

QString processPostText(QString text, const QString &boardName, Database::RefMap *referencedPosts, quint64 deletedPost,
                        MarkupLanguage languages)
{
    init_once(QString, langs, QString()) {
        QStringList sl = Tools::supportedCodeLanguages();
        sl.removeAll("url");
        langs = sl.join("|").replace("+", "\\+");
    }
    text.replace(QRegExp("\r+\n"), "\n");
    text.replace("\r", "\n");
    ProcessingInfo info(text, boardName, referencedPosts, deletedPost);
    if (ExtendedWakabaMarkLanguage & languages) {
        process(info, &convertMonospace, QRegExp("``", Qt::CaseInsensitive, QRegExp::FixedString), false, true);
        process(info, &convertNomarkup, QRegExp("''", Qt::CaseInsensitive, QRegExp::FixedString), false, true);
        process(info, &convertPre, QRegExp("/\\-\\-pre\\s+", Qt::CaseInsensitive),
                QRegExp("\\s+\\\\\\-\\-", Qt::CaseInsensitive));
        process(info, &convertCode, QRegExp("/\\-\\-code\\s+(" + langs + ")\\s+", Qt::CaseInsensitive),
                QRegExp("\\s+\\\\\\-\\-", Qt::CaseInsensitive));
    }
    if (BBCodeLanguage & languages) {
        process(info, &convertPre, QRegExp("[pre]", Qt::CaseInsensitive, QRegExp::FixedString),
                QRegExp("[/pre]", Qt::CaseInsensitive, QRegExp::FixedString));
        process(info, &convertCode, QRegExp("[code]", Qt::CaseInsensitive, QRegExp::FixedString),
                QRegExp("[/code]", Qt::CaseInsensitive, QRegExp::FixedString));
        process(info, &convertCode, QRegExp("\\[code\\s+lang\\=\"?(" + langs + ")\"?\\s*\\]", Qt::CaseInsensitive),
                QRegExp("[/code]", Qt::CaseInsensitive, QRegExp::FixedString));
        process(info, &convertCode, QRegExp("\\[(" + langs + ")\\]", Qt::CaseInsensitive),
                QRegExp("\\[/(" + langs + ")\\]", Qt::CaseInsensitive), false, false, &checkLangsMatch);
        process(info, &convertMonospace, QRegExp("[m]", Qt::CaseInsensitive, QRegExp::FixedString),
                QRegExp("[/m]", Qt::CaseInsensitive, QRegExp::FixedString));
        process(info, &convertNomarkup, QRegExp("[n]", Qt::CaseInsensitive, QRegExp::FixedString),
                QRegExp("[/n]", Qt::CaseInsensitive, QRegExp::FixedString));
    }
    if ((ExtendedWakabaMarkLanguage & languages) || (BBCodeLanguage & languages)) {
        process(info, &convertExternalLink, QRegExp(Tools::externalLinkRegexpPattern()), QRegExp(), false, false,
                &checkExternalLink);
        process(info, &convertProtocol, QRegExp("(mailto|irc|news):(\\S+)"), QRegExp());
        processStrikedOutShitty(info);
        processStrikedOutShittyWord(info);
        process(info, &convertTooltipShitty, QRegExp("([^\\?\\s]+)\\?{3}\"([^\"]+)\""), QRegExp());
        process(info, &convertPostLink, QRegExp(">>([1-9][0-9]*)"), QRegExp());
        QString boards = AbstractBoard::boardNames().join("|");
        process(info, &convertPostLink, QRegExp(">>/(" + boards + ")/([1-9][0-9]*)"), QRegExp());
        process(info, &convertMarkup, QRegExp("----", Qt::CaseInsensitive, QRegExp::FixedString), QRegExp());
    }
    if (ExtendedWakabaMarkLanguage & languages)
        process(info, &convertMarkup, QRegExp("---", Qt::CaseInsensitive, QRegExp::FixedString));
    if ((ExtendedWakabaMarkLanguage & languages) || (BBCodeLanguage & languages))
        process(info, &convertMarkup, QRegExp("--", Qt::CaseInsensitive, QRegExp::FixedString), QRegExp());
    if (ExtendedWakabaMarkLanguage & languages) {
        process(info, &convertMarkup, QRegExp("***", Qt::CaseInsensitive, QRegExp::FixedString));
        process(info, &convertMarkup, QRegExp("**", Qt::CaseInsensitive, QRegExp::FixedString));
        process(info, &convertMarkup, QRegExp("*", Qt::CaseInsensitive, QRegExp::FixedString));
        process(info, &convertMarkup, QRegExp("___", Qt::CaseInsensitive, QRegExp::FixedString));
        process(info, &convertMarkup, QRegExp("__", Qt::CaseInsensitive, QRegExp::FixedString));
        process(info, &convertMarkup, QRegExp("_", Qt::CaseInsensitive, QRegExp::FixedString));
        process(info, &convertMarkup, QRegExp("///", Qt::CaseInsensitive, QRegExp::FixedString));
        process(info, &convertCSpoiler, QRegExp("%%%", Qt::CaseInsensitive, QRegExp::FixedString));
        process(info, &convertMarkup, QRegExp("%%", Qt::CaseInsensitive, QRegExp::FixedString));
    }
    if (BBCodeLanguage & languages) {
        process(info, &convertMarkup, QRegExp("[b]", Qt::CaseInsensitive, QRegExp::FixedString),
                QRegExp("[/b]", Qt::CaseInsensitive, QRegExp::FixedString), true);
        process(info, &convertMarkup, QRegExp("[i]", Qt::CaseInsensitive, QRegExp::FixedString),
                QRegExp("[/i]", Qt::CaseInsensitive, QRegExp::FixedString), true);
        process(info, &convertMarkup, QRegExp("[s]", Qt::CaseInsensitive, QRegExp::FixedString),
                QRegExp("[/s]", Qt::CaseInsensitive, QRegExp::FixedString), true);
        process(info, &convertMarkup, QRegExp("[u]", Qt::CaseInsensitive, QRegExp::FixedString),
                QRegExp("[/u]", Qt::CaseInsensitive, QRegExp::FixedString), true);
        process(info, &convertMarkup, QRegExp("[sub]", Qt::CaseInsensitive, QRegExp::FixedString),
                QRegExp("[/sub]", Qt::CaseInsensitive, QRegExp::FixedString), true);
        process(info, &convertMarkup, QRegExp("[sup]", Qt::CaseInsensitive, QRegExp::FixedString),
                QRegExp("[/sup]", Qt::CaseInsensitive, QRegExp::FixedString), true);
        process(info, &convertMarkup, QRegExp("[spoiler]", Qt::CaseInsensitive, QRegExp::FixedString),
                QRegExp("[/spoiler]", Qt::CaseInsensitive, QRegExp::FixedString), true);
        process(info, &convertUrl, QRegExp("[url]", Qt::CaseInsensitive, QRegExp::FixedString),
                QRegExp("[/url]", Qt::CaseInsensitive, QRegExp::FixedString), true);
        process(info, &convertCSpoiler, QRegExp("[cspoiler]", Qt::CaseInsensitive, QRegExp::FixedString),
                QRegExp("[/cspoiler]", Qt::CaseInsensitive, QRegExp::FixedString), true);
        process(info, &convertCSpoiler, QRegExp("\\[cspoiler\\s+title\\=\"([^\"]*)\"\\s*\\]", Qt::CaseInsensitive),
                QRegExp("[/cspoiler]", Qt::CaseInsensitive, QRegExp::FixedString), true);
        process(info, &convertTooltip, QRegExp("\\[tooltip\\s+value\\=\"([^\"]*)\"\\s*\\]", Qt::CaseInsensitive),
                QRegExp("[/tooltip]", Qt::CaseInsensitive, QRegExp::FixedString), true);
        process(info, &convertUnorderedList, QRegExp("\\[ul\\s+type\\=\"?(disc|circle|square|d|c|s)\"?\\s*\\]",
                                                     Qt::CaseInsensitive),
                QRegExp("[/ul]", Qt::CaseInsensitive, QRegExp::FixedString), true);
        process(info, &convertOrderedList, QRegExp("[ol]", Qt::CaseInsensitive, QRegExp::FixedString),
                QRegExp("[/ol]", Qt::CaseInsensitive, QRegExp::FixedString), true);
        process(info, &convertListItem, QRegExp("\\[li(\\s+value\\=\"?(\\d+)\"?\\s*)?\\]", Qt::CaseInsensitive),
                QRegExp("[/li]", Qt::CaseInsensitive, QRegExp::FixedString), true);
    }
    if ((ExtendedWakabaMarkLanguage & languages) || (BBCodeLanguage & languages)) {
        process(info, &convertCitation, QRegExp(">"), QRegExp("\n|$"), false, false, &checkNotInterrupted);
    }
    return info.toHtml();
}

QString toHtml(const QString &s)
{
    QString ss;
    int last = 0;
    QRegExp rx(Tools::externalLinkRegexpPattern());
    int ind = rx.indexIn(s);
    while (ind >= 0) {
        if (rx.cap(2).count('.') != 3 && !Tools::externalLinkRootZoneExists(rx.cap(4))) {
            ind = rx.indexIn(s, ind + rx.matchedLength());
            continue;
        }
        ss += BTextTools::toHtml(s.mid(last, ind - last), false);
        ss += "<a href=\"";
        QString href = rx.cap();
        if (!href.startsWith("http") && !href.startsWith("ftp"))
            href.prepend("http://");
        ss += href + "\">" + rx.cap() + "</a>";
        last = ind + rx.matchedLength();
        ind = rx.indexIn(s, ind + rx.matchedLength());
    }
    ss += BTextTools::toHtml(s.mid(last), false);
    return ss;
}

void toHtml(QString *s)
{
    if (!s)
        return;
    *s = toHtml(*s);
}

}
