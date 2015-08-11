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

struct ProcessPostTextContext;

typedef QPair<int, int> SkipPair;
typedef QList<SkipPair> SkipList;
typedef void (*ProcessPostTextFunction)(ProcessPostTextContext &c);

static void processPostText(ProcessPostTextContext &c, const SkipList &skip, ProcessPostTextFunction next);

struct ProcessPostTextContext
{
    QString &text;
    const int start;
    const int length;
    const QString &boardName;
    Database::RefMap *referencedPosts;
    quint64 deletedPost;
public:
    explicit ProcessPostTextContext(QString &txt, const QString &board, Database::RefMap *refPosts = 0,
                                    quint64 delPost = 0) :
        text(txt), start(0), length(txt.length()), boardName(board), referencedPosts(refPosts), deletedPost(delPost)
    {
        //
    }
    explicit ProcessPostTextContext(const ProcessPostTextContext &c, int s, int l) :
        text(c.text), start(s), length(l), boardName(c.boardName), referencedPosts(c.referencedPosts),
        deletedPost(c.deletedPost)
    {
        //
    }
    explicit ProcessPostTextContext(const ProcessPostTextContext &c, QString &txt) :
        text(txt), start(0), length(txt.length()), boardName(c.boardName), referencedPosts(c.referencedPosts),
        deletedPost(c.deletedPost)
    {
        //
    }
public:
    void addReferencedPost(const QString &board, quint64 postNumber, quint64 threadNumber)
    {
        if (!referencedPosts || board.isEmpty() || !postNumber || !threadNumber)
            return;
        referencedPosts->insert(Database::RefKey(board, postNumber), threadNumber);
    }
    bool isValid() const
    {
        return (start >= 0 && length > 0 && (start + length) <= text.length());
    }
    QString mid() const
    {
        return isValid() ? text.mid(start, length) : QString();
    }
    void process(QString &txt, const SkipList &skip, ProcessPostTextFunction f = 0)
    {
        ProcessPostTextContext cc(*this, txt);
        processPostText(cc, skip, f);
        replaceWith(txt);
    }
    void replaceWith(const QString &s)
    {
        if (!isValid())
            return;
        text.replace(start, length, s);
    }
};

static QString tagName(const QRegExp &rx)
{
    QString cl = rx.cap();
    cl = cl.mid(1, cl.length() - 2);
    int ind = cl.indexOf(' ');
    if (ind >= 0)
        cl = cl.left(ind);
    return cl;
}

static SkipList externalLinks(const QString &s)
{
    QRegExp rx(Tools::externalLinkRegexpPattern());
    int ind = rx.indexIn(s);
    SkipList skip;
    while (ind >= 0) {
        if (rx.cap(2).count('.') == 3 || Tools::externalLinkRootZoneExists(rx.cap(4)))
            skip << qMakePair(ind, rx.matchedLength());
        ind = rx.indexIn(s, ind + rx.matchedLength());
    }
    return skip;
}

static bool in(const SkipList &skip, int index)
{
    if (index < 0 || skip.isEmpty())
        return false;
    foreach (const SkipPair &p, skip) {
        if (index >= p.first && index <= (p.first + p.second))
            return true;
    }
    return false;
}

static void toHtml(ProcessPostTextContext &c)
{
    if (!c.isValid())
        return;
    c.text.replace(c.start, c.length, BTextTools::toHtml(c.text.mid(c.start, c.length), false));
}

static void processPostText(ProcessPostTextContext &c, const SkipList &skip, ProcessPostTextFunction next)
{
    if (!c.isValid() || !next)
        return;
    if (!skip.isEmpty()) {
        const SkipPair &p = skip.last();
        int start = p.first + p.second;
        ProcessPostTextContext cc(c, start, c.text.length() - start);
        next(cc);
    }
    foreach (int i, bRangeR(skip.size() - 1, 0)) {
        const SkipPair &p = skip.at(i);
        int start = 0;
        if (i > 0) {
            const SkipPair &pp = skip.at(i - 1);
            start = pp.first + pp.second;
        }
        ProcessPostTextContext cc(c, start, p.first - start);
        next(cc);
    }
    if (skip.isEmpty()) {
        ProcessPostTextContext cc(c, 0, c.text.length());
        next(cc);
    }
}

static void processAsymmetric(ProcessPostTextContext &c, ProcessPostTextFunction next, const QString &bbTag,
                              const QString &htmlTag, const QString &openTag = QString(), bool quote = false)
{
    if (!c.isValid())
        return;
    SkipList skip;
    QString t = c.mid();
    QString bbop = "[" + bbTag + "]";
    QString bbcl = "[/" + bbTag + "]";
    QString op = !openTag.isEmpty() ? openTag : ("<" + htmlTag + ">");
    int s = t.indexOf(bbop, 0, Qt::CaseInsensitive);
    if (s < 0)
        return c.process(t, skip, next);
    QRegExp rx("(\\[" + bbTag + "\\])|(\\[/" + bbTag + "\\])", Qt::CaseInsensitive);
    int ind = t.indexOf(rx, s + bbop.length());
    int depth = 1;
    while (ind >= 0) {
        if (rx.cap() == bbop)
            ++depth;
        else
            --depth;
        if (!depth) {
            if (quote) {
                int len = ind - s - bbop.length();
                QString tt = BTextTools::toHtml(t.mid(s + bbop.length(), len), true);
                t.replace(ind, bbcl.length(), "</" + htmlTag + ">");
                t.replace(s + bbop.length(), len, tt);
                t.replace(s, bbop.length(), op);
                skip << qMakePair(s, tt.length() + htmlTag.length() + 3 + op.length());
                s = t.indexOf(bbop, s + tt.length() + htmlTag.length() + 3 + op.length(), Qt::CaseInsensitive);
            } else {
                t.replace(ind, bbcl.length(), "</" + htmlTag + ">");
                t.replace(s, bbop.length(), op);
                skip << qMakePair(s, op.length());
                skip << qMakePair(ind + (op.length() - bbop.length()), htmlTag.length() + 3);
                s = t.indexOf(bbop, ind + (op.length() - bbop.length()) + htmlTag.length() + 3, Qt::CaseInsensitive);
            }
            if (s < 0)
                return c.process(t, skip, next);
            depth = 1;
            ind = t.indexOf(rx, s + bbop.length());
        } else {
            ind = t.indexOf(rx, ind + rx.matchedLength());
        }
    }
    c.process(t, skip, next);
}

static void processSimmetric(ProcessPostTextContext &c, ProcessPostTextFunction next, const QString &wmTag1,
                             const QString &wmTag2, const QString &htmlTag, const QString &openTag = QString(),
                             bool quote = false)
{
    if (!c.isValid())
        return;
    SkipList skip;
    QString t = c.mid();
    int i = 0;
    int s = -1;
    QChar last = '\0';
    SkipList links = externalLinks(t);
    QString op = !openTag.isEmpty() ? openTag : ("<" + htmlTag + ">");
    while (i < t.length()) {
        if (i <= (t.length() - wmTag1.length()) && (t.mid(i, wmTag1.length()) == wmTag1
                                                  || (t.mid(i, wmTag1.length()) == wmTag2 && !in(links, i)))) {
            if (s >= 0 && t.at(i) == last) {
                if (quote) {
                    int len = i - s - wmTag1.length();
                    QString tt = BTextTools::toHtml(t.mid(s + wmTag1.length(), len), true);
                    t.replace(i, wmTag1.length(), "</" + htmlTag + ">");
                    t.replace(s + wmTag1.length(), len, tt);
                    t.replace(s, wmTag1.length(), op);
                    skip << qMakePair(s, tt.length() + htmlTag.length() + 3 + op.length());
                } else {
                    t.replace(i, wmTag1.length(), "</" + htmlTag + ">");
                    t.replace(s, wmTag1.length(), op);
                    skip << qMakePair(s, op.length());
                    skip << qMakePair(i + (op.length() - wmTag1.length()), htmlTag.length() + 3);
                }
                s = -1;
                last = '\0';
            } else if (QChar('\0') == last) {
                s = i;
                last = t.at(i);
            }
            i += wmTag1.length();
        } else {
            ++i;
        }
    }
    c.process(t, skip, next);
}

static void processWakabaMarkReplaceDoubleDash(ProcessPostTextContext &c)
{
    if (!c.isValid())
        return;
    QString t = c.mid();
    t.replace("--", "\u2013");
    c.process(t, SkipList(), &toHtml);
}

static void processWakabaMarkExternalLink(ProcessPostTextContext &c)
{
    if (!c.isValid())
        return;
    SkipList skip;
    QString t = c.mid();
    QRegExp rx(Tools::externalLinkRegexpPattern());
    int ind = rx.indexIn(t);
    while (ind >= 0) {
        if (rx.cap(2).count('.') != 3 && !Tools::externalLinkRootZoneExists(rx.cap(4))) {
            ind = rx.indexIn(t, ind + rx.matchedLength());
            continue;
        }
        QString cap = rx.cap();
        if (!cap.startsWith("http") && !cap.startsWith("ftp"))
            cap.prepend("http://");
        int ml = rx.matchedLength();
        if (cap.endsWith('.')) {
            cap.remove(cap.length() - 1, 1);
            ml -= 1;
        }
        t.insert(ind + ml, "</a>");
        t.insert(ind, "<a href=\"" + cap + "\">");
        skip << qMakePair(ind, cap.length() + 11);
        skip << qMakePair(ind + ml + cap.length() + 11, 4);
        ind = rx.indexIn(t, ind + ml + cap.length() + 15);
    }
    c.process(t, skip, &processWakabaMarkReplaceDoubleDash);
}

static void processWakabaMarkLink(ProcessPostTextContext &c)
{
    if (!c.isValid())
        return;
    SkipList skip;
    QString t = c.mid();
    QRegExp rx(">>(/(" + AbstractBoard::boardNames().join("|") + ")/)?\\d+");
    int ind = rx.indexIn(t);
    QString prefix = SettingsLocker()->value("Site/path_prefix").toString();
    while (ind >= 0) {
        QString cap = rx.cap();
        QString postNumber = cap.mid(2);
        QString boardName = c.boardName;
        if (postNumber.startsWith('/')) {
            int secondSlash = postNumber.indexOf('/', 1);
            boardName = postNumber.mid(1, secondSlash - 1);
            postNumber.remove(0, secondSlash + 1);
        }
        bool ok = false;
        quint64 pn = postNumber.toULongLong(&ok);
        quint64 tn = 0;
        if (ok && pn && (pn != c.deletedPost) && Database::postExists(boardName, pn, &tn)) {
            QString threadNumber = QString::number(tn);
            QString href = "href=\"/" + prefix + boardName + "/thread/" + threadNumber + ".html#" + postNumber + "\"";
            QString a = "<a " + href + ">" + cap.replace(">", "&gt;") + "</a>";
            t.replace(ind, rx.matchedLength(), a);
            skip << qMakePair(ind, a.length());
            ind = rx.indexIn(t, ind + a.length());
            c.addReferencedPost(boardName, pn, tn);
        } else {
            ind = rx.indexIn(t, ind + rx.matchedLength());
        }
    }
    c.process(t, skip, &processWakabaMarkExternalLink);
}

static void processWakabaMarkMailto(ProcessPostTextContext &c)
{
    if (!c.isValid())
        return;
    SkipList skip;
    QString t = c.mid();
    QRegExp rx("[\\w\\.\\-]+@([\\w\\.\\-]+\\.[a-z]{2,6})");
    QRegExp rxLink(Tools::externalLinkRegexpPattern());
    int ind = rx.indexIn(t);
    while (ind >= 0) {
        if (rxLink.cap(2).count('.') != 3 && !Tools::externalLinkRootZoneExists(rx.cap(4))) {
            ind = rx.indexIn(t, ind + rx.matchedLength());
            continue;
        }
        QString mail = rx.cap();
        if (!rxLink.exactMatch(rx.cap(1))) {
            ind = rx.indexIn(t, ind + rx.matchedLength());
            continue;
        }
        QString result = "<a href=\"mailto:" + mail + "\">" + BTextTools::toHtml(mail) + "</a>";
        t.replace(ind, rx.matchedLength(), result);
        skip << qMakePair(ind, result.length());
        ind = rx.indexIn(t, ind + result.length());
    }
    c.process(t, skip, &processWakabaMarkLink);
}

static void processWakabaMarkTooltip(ProcessPostTextContext &c)
{
    if (!c.isValid())
        return;
    SkipList skip;
    QString t = c.mid();
    QRegExp rx("\\S+\\?{3}\"(.*)\"");
    rx.setMinimal(true);
    int ind = rx.indexIn(t);
    while (ind >= 0) {
        QString tooltip = rx.cap(1);
        int ttind = t.indexOf(tooltip, ind) - 4;
        t.replace(ttind, tooltip.length() + 5, "</span>");
        QString op = "<span class=\"tooltip\" title=\"" + tooltip + "\">";
        t.insert(ind, op);
        skip << qMakePair(ind, op.length());
        skip << qMakePair(ttind + op.length(), 7);
        ind = rx.indexIn(t, ttind + op.length() + 7);
    }
    c.process(t, skip, &processWakabaMarkMailto);
}

static void processWakabaMarkStrikeoutShitty(ProcessPostTextContext &c)
{
    if (!c.isValid())
        return;
    SkipList skip;
    QString t = c.mid();
    QRegExp rx("(\\^H)+");
    int ind = rx.indexIn(t);
    while (ind >= 0) {
        int s = ind - (rx.matchedLength() / 2);
        if (s < 0)
            break;
        t.replace(ind, rx.matchedLength(), "</s>");
        t.insert(s, "<s>");
        skip << qMakePair(s, 3);
        skip << qMakePair(ind + 3, 4);
        ind = rx.indexIn(t, ind + 7);
    }
    c.process(t, skip, &processWakabaMarkTooltip);
}

static void processWakabaMarkStrikeout(ProcessPostTextContext &c)
{
    processSimmetric(c, &processWakabaMarkStrikeoutShitty, "---", "", "s");
}

static void processWakabaMarkReplaceQuadripleDash(ProcessPostTextContext &c)
{
    if (!c.isValid())
        return;
    QString t = c.mid();
    t.replace("----", "\u2014");
    c.process(t, SkipList(), &processWakabaMarkStrikeout);
}

static void processWakabaMarkItalicExtra(ProcessPostTextContext &c)
{
    processSimmetric(c, &processWakabaMarkReplaceQuadripleDash, "///", "", "em");
}

static void processWakabaMarkItalic(ProcessPostTextContext &c)
{
    processSimmetric(c, &processWakabaMarkItalicExtra, "*", "_", "em");
}

static void processWakabaMarkBold(ProcessPostTextContext &c)
{
    processSimmetric(c, &processWakabaMarkItalic, "**", "__", "strong");
}

static void processWakabaMarkUnderlined(ProcessPostTextContext &c)
{
    processSimmetric(c, &processWakabaMarkBold, "***", "___", "u");
}

static void processWakabaMarkSpoiler(ProcessPostTextContext &c)
{
    processSimmetric(c, &processWakabaMarkUnderlined, "%%", "", "span", "<span class=\"spoiler\">");
}

static void processWakabaMarkCollapsibleSpoiler(ProcessPostTextContext &c)
{
    if (!c.isValid())
        return;
    SkipList skip;
    QString t = c.mid();
    int i = 0;
    int s = -1;
    QChar last = '\0';
    QString op = "<span class=\"cspoiler\"><span class=\"cspoilerTitle\" "
            "onclick=\"lord.expandCollapseSpoiler(this);\">Spoiler</span><span class=\"cspoilerBody\" "
            "style=\"display: none;\">";
    while (i < t.length()) {
        if (i <= (t.length() - 3) && t.mid(i, 3) == "%%%") {
            if (s >= 0 && t.at(i) == last) {
                t.replace(i, 3, "</span></span>");
                t.replace(s, 3, op);
                skip << qMakePair(s, op.length());
                skip << qMakePair(i + (op.length() - 3), 14 + 3);
                s = -1;
                last = '\0';
            } else if (QChar('\0') == last) {
                s = i;
                last = t.at(i);
            }
            i += 3;
        } else {
            ++i;
        }
    }
    c.process(t, skip, &processWakabaMarkSpoiler);
}

static void processTagUrl(ProcessPostTextContext &c)
{
    if (!c.isValid())
        return;
    SkipList skip;
    QString t = c.mid();
    QRegExp rx("\\[url\\](.+)\\[/url\\]");
    rx.setMinimal(true);
    QRegExp rxLink(Tools::externalLinkRegexpPattern());
    int ind = rx.indexIn(t);
    while (ind >= 0) {
        QString href = rx.cap(1);
        if (!rxLink.exactMatch(href)) {
            ind = rx.indexIn(t, ind + rx.matchedLength());
            continue;
        }
        if (rxLink.cap(2).count('.') != 3 && !Tools::externalLinkRootZoneExists(rxLink.cap(4))) {
            ind = rx.indexIn(t, ind + rx.matchedLength());
            continue;
        }
        QString hrefold = href;
        if (!href.startsWith("http") && !href.startsWith("ftp"))
            href.prepend("http://");
        QString result = "<a href=\"" + href + "\">" + BTextTools::toHtml(hrefold) + "</a>";
        t.replace(ind, rx.matchedLength(), result);
        skip << qMakePair(ind, result.length());
        ind = rx.indexIn(t, ind + result.length());
    }
    c.process(t, skip, &processWakabaMarkCollapsibleSpoiler);
}

static void processTags(ProcessPostTextContext &c)
{
    typedef QMap<QString, QString> StringMap;
    init_once(StringMap, tags, StringMap()) {
        tags.insert("[b]", "[/b]");
        tags.insert("[i]", "[/i]");
        tags.insert("[s]", "[/s]");
        tags.insert("[u]", "[/u]");
        tags.insert("[spoiler]", "[/spoiler]");
        tags.insert("[sub]", "[/sub]");
        tags.insert("[sup]", "[/sup]");
    }
    typedef QPair<QString, QString> StringPair;
    typedef QMap<QString, StringPair> StringPairMap;
    init_once(StringPairMap, htmls, StringPairMap()) {
        htmls.insert("[b]", qMakePair(QString("<strong>"), QString("</strong>")));
        htmls.insert("[i]", qMakePair(QString("<em>"), QString("</em>")));
        htmls.insert("[s]", qMakePair(QString("<s>"), QString("</s>")));
        htmls.insert("[u]", qMakePair(QString("<u>"), QString("</u>")));
        htmls.insert("[spoiler]", qMakePair(QString("<span class=\"spoiler\">"), QString("</span>")));
        htmls.insert("[sub]", qMakePair(QString("<sub>"), QString("</sub>")));
        htmls.insert("[sup]", qMakePair(QString("<sup>"), QString("</sup>")));
    }
    ProcessPostTextFunction next = &processTagUrl;
    if (!c.isValid())
        return;
    SkipList skip;
    QString t = c.mid();
    QRegExp rxop(QStringList(tags.keys()).join("|").replace("[", "\\[").replace("]", "\\]"), Qt::CaseInsensitive);
    int s = t.indexOf(rxop);
    if (s < 0)
        return c.process(t, skip, next);
    QString bbop = rxop.cap();
    QString bbcl = tags.value(bbop);
    StringPair html = htmls.value(bbop);
    QRegExp rx(QString(bbop + "|" + bbcl).replace("[", "\\[").replace("]", "\\]"), Qt::CaseInsensitive);
    int ind = t.indexOf(rx, s + bbop.length());
    int depth = 1;
    while (ind >= 0) {
        if (rx.cap() == bbop)
            ++depth;
        else
            --depth;
        if (!depth) {
            t.replace(ind, bbcl.length(), html.second);
            t.replace(s, bbop.length(), html.first);
            skip << qMakePair(s, html.first.length());
            skip << qMakePair(ind + (html.first.length() - bbop.length()), html.second.length());
            s = t.indexOf(bbop, ind + (html.first.length() - bbop.length()) +  html.second.length(),
                          Qt::CaseInsensitive);
            next = &processTags;
            if (s < 0)
                break;
            depth = 1;
            ind = t.indexOf(rx, s + bbop.length());
        } else {
            ind = t.indexOf(rx, ind + rx.matchedLength());
        }
    }
    c.process(t, skip, next);
}

static void processWakabaMarkQuote(ProcessPostTextContext &c)
{
    if (!c.isValid())
        return;
    SkipList skip;
    QString t = c.mid();
    QRegExp rx(">[^\n>][^\n]+");
    int ind = rx.indexIn(t);
    while (ind >= 0) {
        if (!ind || t.at(ind - 1) == '\n') {
            t.insert(ind + rx.matchedLength(), "</span>");
            t.insert(ind, "<span class=\"quotation\">");
            skip << qMakePair(ind, 24);
            skip << qMakePair(ind + rx.matchedLength() + 24, 7);
            ind = rx.indexIn(t, ind + rx.matchedLength() + 31);
        } else {
            ind = rx.indexIn(t, ind + rx.matchedLength());
        }
    }
    c.process(t, skip, &processTags);
}

static void processWakabaMarkList(ProcessPostTextContext &c)
{
    typedef QMap<int, QString> ListTypeMap;
    init_once(ListTypeMap, listTypes, ListTypeMap()) {
        listTypes.insert(1, "disc");
        listTypes.insert(2, "circle");
        listTypes.insert(3, "square");
    }
    if (!c.isValid())
        return;
    SkipList skip;
    QString t = c.mid();
    bool cr = t.contains('\r');
    QStringList sl = t.split(QRegExp("\r?\n"));
    int offset = 0;
    int last = 0;
    foreach (int i, bRangeD(0, sl.size() - 1)) {
        QString &s = sl[i];
        QRegExp rx("^\\d+\\. ");
        int current = 0;
        if (s.startsWith("* "))
            current = 1;
        else if (s.startsWith("+ "))
            current = 2;
        else if (s.startsWith("- "))
            current = 3;
        else if (!rx.indexIn(s))
            current = 4;
        if (current) {
            if (current == last) {
                if (current < 4) {
                    s.append("</li>");
                    s.replace(0, 2, "<li>");
                    skip << qMakePair(offset, 4);
                    skip << qMakePair(offset + s.length() - 5, 5);
                } else {
                    s.append("</li>");
                    QString ss = "<li value=\"" + rx.cap().left(rx.cap().length() - 2) + "\">";
                    s.replace(0, rx.matchedLength(), ss);
                    skip << qMakePair(offset, ss.length());
                    skip << qMakePair(offset + s.length() - 5, 5);
                }
            } else {
                int k = 0;
                if (last) {
                    if (last < 4)
                        s.prepend("</ul>");
                    else
                        s.prepend("</ol>");
                    k = 5;
                }
                if (current < 4) {
                    s.append("</li>");
                    QString ss = "<ul type=\"" + listTypes.value(current) + "\"><li>";
                    s.replace(k, 2, ss);
                    skip << qMakePair(offset, ss.length() + k);
                    skip << qMakePair(offset + s.length() - 5, 5);
                } else {
                    s.append("</li>");
                    QString ss = "<ol><li value=\"" + rx.cap().left(rx.cap().length() - 2) + "\">";
                    s.replace(k, rx.matchedLength(), ss);
                    skip << qMakePair(offset, ss.length() + k);
                    skip << qMakePair(offset + s.length() - 5, 5);
                }
            }
        } else if (last) {
            if (last < 4)
                s.prepend("</ul>");
            else
                s.prepend("</ol>");
            skip << qMakePair(offset, 5);
        }
        offset += s.length() + (cr ? 2 : 1);
        last = current;
    }
    if (last) {
        if (last < 4)
            sl.last().append("</ul>");
        else
            sl.last().append("</ol>");
        skip.last().second += 5;
    }
    t = sl.join(cr ? "\r\n" : "\n");
    c.process(t, skip, &processWakabaMarkQuote);
}

static void processTagTooltip(ProcessPostTextContext &c)
{
    if (!c.isValid())
        return;
    SkipList skip;
    QString t = c.mid();
    QRegExp rx("\\[tooltip\\s+value\\=\"(.*)\"\\s*\\]");
    rx.setMinimal(true);
    int indStart = rx.indexIn(t);
    int indEnd = t.indexOf("[/tooltip]", indStart + rx.matchedLength());
    while (indStart >= 0 && indEnd > 0) {
        QString tooltip = rx.cap(1);
        t.replace(indEnd, 10, "</span>");
        QString op = "<span class=\"tooltip\" title=\"" + tooltip + "\">";
        t.replace(indStart, rx.matchedLength(), op);
        skip << qMakePair(indStart, op.length());
        skip << qMakePair(indEnd + (op.length() - rx.matchedLength()), 7);
        indStart = rx.indexIn(t, indEnd + (op.length() - rx.matchedLength()));
        indEnd = t.indexOf("[/tooltip]", indStart + rx.matchedLength());
    }
    c.process(t, skip, &processWakabaMarkList);
}

static void processWakabaMarkMonospaceSingle(ProcessPostTextContext &c)
{
    processSimmetric(c, &processTagTooltip, "`", "", "font", "<font face=\"monospace\">", true);
}

static void processTagCode(ProcessPostTextContext &c)
{
    if (!c.isValid())
        return;
    SkipList skip;
    QString t = c.mid();
    QString srchighlightPath = BDirTools::findResource("srchilite", BDirTools::AllResources);
    if (!srchighlightPath.isEmpty()) {
        QStringList langs = Tools::supportedCodeLanguages();
        langs.removeAll("url");
        QString tags;
        foreach (const QString &s, langs)
            tags += "|\\[" + s + "\\]";
        QRegExp rx("\\[code\\s+lang\\=\"?(" + langs.join("|").replace("+", "\\+") + ")\"?\\s*\\]"
                   + tags.replace("+", "\\+"));
        int indStart = rx.indexIn(t);
        QString tag = tagName(rx);
        int indEnd = t.indexOf("[/" + tag + "]", indStart + rx.matchedLength());
        while (indStart >= 0 && indEnd > 0) {
            QString lang;
            if (langs.contains(tag)) {
                lang = tag;
            } else {
                lang = rx.cap();
                lang.remove(QRegExp("\\[code\\s+lang\\=\"?"));
                lang.remove(QRegExp("\"?\\s*\\]"));
            }
            lang.replace("++", "pp");
            int codeStart = indStart + rx.matchedLength();
            int codeLength = indEnd - codeStart;
            QString code = t.mid(codeStart, codeLength);
            std::istringstream in(Tools::toStd(code));
            std::ostringstream out;
            try {
                srchilite::SourceHighlight sourceHighlight("html.outlang");
                sourceHighlight.setDataDir(Tools::toStd(srchighlightPath));
                sourceHighlight.highlight(in, out, Tools::toStd(lang + ".lang"));
            } catch (const srchilite::ParserException &e) {
                Tools::log("Controller::processTagCode", e);
                return;
            } catch (const srchilite::IOException &e) {
                Tools::log("Controller::processTagCode", e);
                return;
            } catch (const std::exception &e) {
                Tools::log("Controller::processTagCode", e);
                return;
            }
            QString result = "<div class=\"codeBlock\">" + Tools::fromStd(out.str()) + "</div>";
            t.replace(indStart, rx.matchedLength() + code.length() + 7, result);
            skip << qMakePair(indStart, result.length());
            indStart = rx.indexIn(t, indStart + result.length());
            indEnd = t.indexOf("[/" + tag + "]", indStart + rx.matchedLength());
        }
    }
    c.process(t, skip, &processWakabaMarkMonospaceSingle);
}

static void processTagCodeNolang(ProcessPostTextContext &c)
{
    processAsymmetric(c, &processTagCode, "code", "pre");
}

static void processTagCspoiler(ProcessPostTextContext &c)
{
    if (!c.isValid())
        return;
    SkipList skip;
    QString t = c.mid();
    QRegExp rx("\\[cspoiler\\s+title\\=\"(.*)\"\\s*\\]");
    rx.setMinimal(true);
    int indStart = rx.indexIn(t);
    int indEnd = t.indexOf("[/cspoiler]", indStart + rx.matchedLength());
    while (indStart >= 0 && indEnd > 0) {
        QString title = rx.cap(1);
        t.replace(indEnd, 11, "</span></span>");
        QString op = "<span class=\"cspoiler\"><span class=\"cspoilerTitle\" title=\"Spoiler\" "
                "onclick=\"lord.expandCollapseSpoiler(this);\">" + title
                + "</span><span class=\"cspoilerBody\" style=\"display: none;\">";
        t.replace(indStart, rx.matchedLength(), op);
        skip << qMakePair(indStart, op.length());
        skip << qMakePair(indEnd + (op.length() - rx.matchedLength()), 14);
        indStart = rx.indexIn(t, indEnd + (op.length() - rx.matchedLength()));
        indEnd = t.indexOf("[/cspoiler]", indStart + rx.matchedLength());
    }
    c.process(t, skip, &processTagCodeNolang);
}

static void processTagQuote(ProcessPostTextContext &c)
{
    processAsymmetric(c, &processTagCspoiler, "q", "font", "<font face=\"monospace\">", true);
}

static void processWakabaMarkCode(ProcessPostTextContext &c)
{
    if (!c.isValid())
        return;
    SkipList skip;
    QString t = c.mid();
    QString srchighlightPath = BDirTools::findResource("srchilite", BDirTools::AllResources);
    if (!srchighlightPath.isEmpty()) {
        QStringList langs = Tools::supportedCodeLanguages();
        langs.removeAll("url");
        QRegExp rx("/\\-\\-code\\s+(" + langs.join("|").replace("+", "\\+") + ")\\s+");
        int indStart = rx.indexIn(t);
        QRegExp rxe("\\s+\\\\\\-\\-");
        int indEnd = t.indexOf(rxe, indStart + rx.matchedLength());
        while (indStart >= 0 && indEnd > 0) {
            QString lang = rx.cap();
            lang.remove(QRegExp("/\\-\\-code\\s+"));
            lang.remove(QRegExp("\\s+"));
            lang.replace("++", "pp");
            int codeStart = indStart + rx.matchedLength();
            int codeLength = indEnd - codeStart;
            QString code = t.mid(codeStart, codeLength);
            std::istringstream in(Tools::toStd(code));
            std::ostringstream out;
            try {
                srchilite::SourceHighlight sourceHighlight("html.outlang");
                sourceHighlight.setDataDir(Tools::toStd(srchighlightPath));
                sourceHighlight.highlight(in, out, Tools::toStd(lang + ".lang"));
            } catch (const srchilite::ParserException &e) {
                Tools::log("Controller::processWakabaMarkCode", e);
                return;
            } catch (const srchilite::IOException &e) {
                Tools::log("Controller::processWakabaMarkCode", e);
                return;
            } catch (const std::exception &e) {
                Tools::log("Controller::processWakabaMarkCode", e);
                return;
            }
            QString result = "<div class=\"codeBlock\">" + Tools::fromStd(out.str()) + "</div>";
            t.replace(indStart, rx.matchedLength() + code.length() + rxe.matchedLength(), result);
            skip << qMakePair(indStart, result.length());
            indStart = rx.indexIn(t, indStart + result.length());
            indEnd = t.indexOf(rxe, indStart + rx.matchedLength());
        }
    }
    c.process(t, skip, &processTagQuote);
}

static void processWakabaMarkPre(ProcessPostTextContext &c)
{
    if (!c.isValid())
        return;
    SkipList skip;
    QString t = c.mid();
    QRegExp rx("/\\-\\-pre\\s+");
    int indStart = rx.indexIn(t);
    QRegExp rxe("\\s+\\\\\\-\\-");
    int indEnd = t.indexOf(rxe, indStart + rx.matchedLength());
    while (indStart >= 0 && indEnd > 0) {
        int codeStart = indStart + rx.matchedLength();
        int codeLength = indEnd - codeStart;
        t.replace(indEnd, rxe.matchedLength(), "</pre>");
        t.replace(indStart, rx.matchedLength(), "<pre>");
        skip << qMakePair(indStart, codeLength + 5 + 6);
        indStart = rx.indexIn(t, indStart + codeLength + 5 + 6);
        indEnd = t.indexOf(rxe, indStart + rx.matchedLength());
    }
    c.process(t, skip, &processWakabaMarkCode);
}

static void processWakabaMarkMonospaceDouble(ProcessPostTextContext &c)
{
    processSimmetric(c, &processWakabaMarkPre, "``", "", "font", "<font face=\"monospace\">", true);
}

//TODO: New

class ProcessingInfo
{
public:
    enum SkipType
    {
        NoSkip = 0,
        HtmlSkip,
        CodeSkip
    };
public:
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
public:
    QList<SkipInfo> skipList;
    QString &mtext;
public:
    explicit ProcessingInfo(QString &txt, const QString &boardName, Database::RefMap *referencedPosts,
                            quint64 deletedPost);
public:
    int find(const QRegExp &rx, int from = 0, bool escapable = false);
    void insert(int from, const QString &txt, SkipType type = HtmlSkip);
    void replace(int from, int length, const QString &txt, int correction, SkipType type = HtmlSkip);
    const QString &text() const;
};

static bool isEscaped(const QString &s, int pos);

ProcessingInfo::ProcessingInfo(QString &txt, const QString &boardName, Database::RefMap *referencedPosts,
                               quint64 deletedPost) :
    BoardName(boardName), DeletedPost(deletedPost), ReferencedPosts(referencedPosts), mtext(txt)
{
    //
}

int ProcessingInfo::find(const QRegExp &rx, int from, bool escapable)
{
    int ind = rx.indexIn(mtext, from);
    while (ind >= 0) {
        bool in = false;
        foreach (int i, bRangeD(0, skipList.length() - 1)) {
            const SkipInfo &inf = skipList.at(i);
            if (ind >= inf.from && ind < (inf.from + inf.length)) {
                ind = rx.indexIn(mtext, inf.from + inf.length + 1);
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
        if (from >= inf.from) {
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
        //if (from + length > inf.from)
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

static QString withoutEscaped(const QString &text)
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

typedef bool (*CheckFunction)(const QRegExp &rxOp, const QRegExp &rxCl);
typedef QString (*ConversionFunction)(const QString &text, const QRegExp &rxOp, const QRegExp &rxCl, QString &op,
                                      QString &cl, ProcessingInfo::SkipType &type);

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
    int inde = getIndE(info, rxOp, rxCl, inds, nestable, escapable, nested);
    bool rerun = false;
    while (inds >= 0 && inde > inds) {
        if (checkFunction && !checkFunction(rxOp, rxCl)) {
            inds = info.find(rxOp, inde + rxCl.matchedLength() + 1, escapable);
            inde = getIndE(info, rxOp, rxCl, inds, nestable, escapable, nested);
            continue;
        }
        QString op;
        QString cl;
        ProcessingInfo::SkipType type = ProcessingInfo::NoSkip;
        QString txt = info.text().mid(inds + rxOp.matchedLength(), inde - inds - rxOp.matchedLength());
        int xxx = txt.length();
        txt = conversionFunction(txt, rxOp, rxCl, op, cl, type);
        if (!txt.isEmpty()) {
            if (!cl.isEmpty())
                info.insert(inde + rxCl.matchedLength(), cl);
            info.replace(inds, inde - inds + rxCl.matchedLength(), txt, rxOp.matchedLength(), type);
            if (!op.isEmpty())
                info.insert(inds, op);
            qDebug() << ">>>>>>>>>>>>>>>>>>>>>" << op << cl << (txt.length() - xxx);
            foreach (ProcessingInfo::SkipInfo si, info.skipList) {
                qDebug() << si.from << si.length << si.type << info.text().mid(si.from, si.length);
            }
            qDebug() << "<<<<<<<<<<<<<<<<<<<<<";
            inds = info.find(rxOp, inds + txt.length() + op.length() + cl.length() + 1, escapable);
        } else {
            inds = info.find(rxOp, inde + rxCl.matchedLength() + 1, escapable);
        }
        if (nestable && nested)
            rerun = true;
        inde = getIndE(info, rxOp, rxCl, inds, nestable, escapable, nested);
    }
    if (rerun)
        process(info, conversionFunction, rxOp, rxCl, nestable, escapable, checkFunction);
}

static void process(ProcessingInfo &info, ConversionFunction conversionFunction, const QRegExp &rxOp, bool nestable = false,
                    bool escapable = false, CheckFunction checkFunction = 0)
{
    QRegExp rxCl(rxOp);
    return process(info, conversionFunction, rxOp, rxCl, nestable, escapable, checkFunction);
}

static bool checkLangsMatch(const QRegExp &rxOp, const QRegExp &rxCl)
{
    return !rxOp.cap(1).isEmpty() && rxOp.cap(1) == rxCl.cap(1);
}

static QString convertMonospace(const QString &text, const QRegExp &, const QRegExp &, QString &op, QString &cl,
                                ProcessingInfo::SkipType &type)
{
    op = "<font family=\"monospace\">";
    cl = "</font>";
    type = ProcessingInfo::CodeSkip;
    return BTextTools::toHtml(withoutEscaped(text));
}

static QString convertNomarkup(const QString &text, const QRegExp &, const QRegExp &, QString &, QString &,
                               ProcessingInfo::SkipType &type)
{
    type = ProcessingInfo::CodeSkip;
    return BTextTools::toHtml(withoutEscaped(text));
}

static QString convertPre(const QString &text, const QRegExp &, const QRegExp &, QString &op, QString &cl,
                          ProcessingInfo::SkipType &type)
{
    op = "<pre>";
    cl = "</pre>";
    type = ProcessingInfo::CodeSkip;
    return withoutEscaped(text);
}

static QString convertCode(const QString &text, const QRegExp &rxOp, const QRegExp &, QString &op, QString &cl,
                           ProcessingInfo::SkipType &type)
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

static QString convertMarkup(const QString &text, const QRegExp &rxOp, const QRegExp &, QString &op, QString &cl,
                             ProcessingInfo::SkipType &type)
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
    WrapperPair p = map.value(rxOp.cap());
    if (p.first.isEmpty())
        return "";
    type = ProcessingInfo::NoSkip;
    if ("----" == rxOp.cap())
        return "\u2014";
    else if ("--" == rxOp.cap())
        return "\u2013";
    op = p.first;
    cl = p.second;
    return text;
}

static QString convertUrl(const QString &text, const QRegExp &, const QRegExp &, QString &, QString &,
                          ProcessingInfo::SkipType &type)
{
    if (text.isEmpty())
        return "";
    type = ProcessingInfo::HtmlSkip;
    QString href = text;
    if (!href.startsWith("http") && !href.startsWith("ftp"))
        href.prepend("http://");
    return "<a href=\"" + href + "\">" + BTextTools::toHtml(text) + "</a>";
}

static QString convertCSpoiler(const QString &text, const QRegExp &rxOp, const QRegExp &, QString &op, QString &cl,
                               ProcessingInfo::SkipType &type)
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

static QString convertTooltip(const QString &text, const QRegExp &rxOp, const QRegExp &, QString &op, QString &cl,
                              ProcessingInfo::SkipType &type)
{
    QString tooltip = rxOp.cap(1);
    type = ProcessingInfo::NoSkip;
    op = "<span class=\"tooltip\" title=\"" + tooltip + "\">";
    cl = "</span>";
    return text;
}

QString processPostText(QString text, const QString &boardName, Database::RefMap *referencedPosts, quint64 deletedPost)
{
    init_once(QString, langs, QString()) {
        QStringList sl = Tools::supportedCodeLanguages();
        sl.removeAll("url");
        langs = sl.join("|").replace("+", "\\+");
    }
    text.replace(QRegExp("\r*\n"), "\n");
    ProcessingInfo info(text, boardName, referencedPosts, deletedPost);
    process(info, &convertMonospace, QRegExp("``", Qt::CaseInsensitive, QRegExp::FixedString), false, true);
    process(info, &convertNomarkup, QRegExp("''", Qt::CaseInsensitive, QRegExp::FixedString), false, true);
    process(info, &convertPre, QRegExp("/\\-\\-pre\\s+", Qt::CaseInsensitive),
            QRegExp("\\s+\\\\\\-\\-", Qt::CaseInsensitive));
    process(info, &convertCode, QRegExp("/\\-\\-code\\s+(" + langs + ")\\s+", Qt::CaseInsensitive),
            QRegExp("\\s+\\\\\\-\\-", Qt::CaseInsensitive));
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
    process(info, &convertMarkup, QRegExp("----", Qt::CaseInsensitive, QRegExp::FixedString));
    process(info, &convertMarkup, QRegExp("---", Qt::CaseInsensitive, QRegExp::FixedString));
    process(info, &convertMarkup, QRegExp("--", Qt::CaseInsensitive, QRegExp::FixedString));
    process(info, &convertMarkup, QRegExp("***", Qt::CaseInsensitive, QRegExp::FixedString));
    process(info, &convertMarkup, QRegExp("**", Qt::CaseInsensitive, QRegExp::FixedString));
    process(info, &convertMarkup, QRegExp("*", Qt::CaseInsensitive, QRegExp::FixedString));
    process(info, &convertMarkup, QRegExp("___", Qt::CaseInsensitive, QRegExp::FixedString));
    process(info, &convertMarkup, QRegExp("__", Qt::CaseInsensitive, QRegExp::FixedString));
    process(info, &convertMarkup, QRegExp("_", Qt::CaseInsensitive, QRegExp::FixedString));
    process(info, &convertMarkup, QRegExp("///", Qt::CaseInsensitive, QRegExp::FixedString));
    process(info, &convertCSpoiler, QRegExp("%%%", Qt::CaseInsensitive, QRegExp::FixedString));
    process(info, &convertMarkup, QRegExp("%%", Qt::CaseInsensitive, QRegExp::FixedString));
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
    process(info, &convertTooltip, QRegExp("\\[tooltip\\s+title\\=\"([^\"]*)\"\\s*\\]", Qt::CaseInsensitive),
            QRegExp("[/tooltip]", Qt::CaseInsensitive, QRegExp::FixedString), true);
    //
    //
    return text;
    //ProcessPostTextContext c(text, boardName, referencedPosts, deletedPost);
    //processWakabaMarkMonospaceDouble(c);
    //return text;
}

QString toHtml(const QString &s)
{
    QString ss = s;
    toHtml(&ss);
    return ss;
}

void toHtml(QString *s)
{
    if (!s)
        return;
    SkipList skip;
    QRegExp rx(Tools::externalLinkRegexpPattern());
    int ind = rx.indexIn(*s);
    while (ind >= 0) {
        if (rx.cap(2).count('.') != 3 && !Tools::externalLinkRootZoneExists(rx.cap(4))) {
            ind = rx.indexIn(*s, ind + rx.matchedLength());
            continue;
        }
        s->insert(ind + rx.matchedLength(), "</a>");
        QString cap = rx.cap();
        if (!cap.startsWith("http") && !cap.startsWith("ftp"))
            cap.prepend("http://");
        s->insert(ind, "<a href=\"" + cap + "\">");
        skip << qMakePair(ind, 11 + cap.length() + rx.matchedLength() + 4);
        ind = rx.indexIn(*s, ind + rx.matchedLength() + cap.length() + 15);
    }
    ProcessPostTextContext c(*s, "", 0L);
    processPostText(c, skip, &toHtml);
}

}
