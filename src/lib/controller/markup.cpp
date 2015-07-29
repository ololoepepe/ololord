#include "controller.h"

#include "baseboard.h"
#include "cache.h"
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

namespace Controller
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

void processPostText(ProcessPostTextContext &c, const SkipList &skip, ProcessPostTextFunction next)
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
        if (!cap.startsWith("http"))
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
        if (rx.cap(2).count('.') != 3 && !Tools::externalLinkRootZoneExists(rx.cap(4))) {
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
        if (rx.cap(2).count('.') != 3 && !Tools::externalLinkRootZoneExists(rxLink.cap(4))) {
            ind = rx.indexIn(t, ind + rx.matchedLength());
            continue;
        }
        QString hrefold = href;
        if (!href.startsWith("http"))
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

QString processPostText(QString text, const QString &boardName, Database::RefMap *referencedPosts, quint64 deletedPost)
{
    ProcessPostTextContext c(text, boardName, referencedPosts, deletedPost);
    processWakabaMarkMonospaceDouble(c);
    return text;
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
        if (!cap.startsWith("http"))
            cap += "http://";
        s->insert(ind, "<a href=\"" + cap + "\">");
        skip << qMakePair(ind, 11 + cap.length() + rx.matchedLength() + 4);
        ind = rx.indexIn(*s, ind + rx.matchedLength() + cap.length() + 15);
    }
    ProcessPostTextContext c(*s, "", 0L);
    processPostText(c, skip, &toHtml);
}

QList<Content::Post> getNewPosts(const cppcms::http::request &req, const QString &boardName, quint64 threadNumber,
                                 quint64 lastPostNumber, bool *ok, QString *error)
{
    AbstractBoard::LockingWrapper board = AbstractBoard::board(boardName);
    TranslatorQt tq(req);
    if (board.isNull()) {
        return bRet(ok, false, error, tq.translate("getNewPosts", "Invalid board name", "error"),
                    QList<Content::Post>());
    }
    bool b = false;
    QList<Post> posts = Database::getNewPosts(req, boardName, threadNumber, lastPostNumber, &b, error);
    if (!b)
        return bRet(ok, false, QList<Content::Post>());
    QList<Content::Post> list;
    foreach (const Post &p, posts) {
        list << board->toController(p, req, &b, error);
        if (!b)
            return bRet(ok, false, QList<Content::Post>());
        if (!list.last().number) {
            return bRet(ok, false, error, tq.translate("getNewPosts", "Internal logic error", "error"),
                        QList<Content::Post>());
        }
    }
    return bRet(ok, true, error, QString(), list);
}

Content::Post getPost(const cppcms::http::request &req, const QString &boardName, quint64 postNumber, bool *ok,
                      QString *error)
{
    AbstractBoard::LockingWrapper board = AbstractBoard::board(boardName);
    TranslatorQt tq(req);
    if (board.isNull())
        return bRet(ok, false, error, tq.translate("getPost", "Invalid board name", "error"), Content::Post());
    bool b = false;
    Post post = Database::getPost(req, boardName, postNumber, &b, error);
    if (!b)
        return bRet(ok, false, Content::Post());
    Content::Post p = board->toController(post, req, &b, error);
    if (!b)
        return bRet(ok, false, Content::Post());
    if (!p.number)
        return bRet(ok, false, error, tq.translate("getPost", "Internal logic error", "error"), Content::Post());
    return bRet(ok, true, error, QString(), p);
}

}
