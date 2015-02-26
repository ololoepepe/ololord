#include "controller.h"

#include "baseboard.h"
#include "cache.h"
#include "database.h"
#include "settingslocker.h"
#include "stored/registereduser.h"
#include "stored/thread.h"
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

typedef QPair<int, int> SkipPair;
typedef QList<SkipPair> SkipList;
typedef void (*ProcessPostTextFunction)(QString &text, int start, int length, const QString &boardName,
                                        quint64 threadNumber, bool processCode);

static const QString ExternalLinkRegexpPattern =
        "(https?:\\/\\/)?([\\w\\.\\-]+)\\.([a-z]{2,6}\\.?)(\\/[\\w\\.\\-]*)*\\/?";

static SkipList externalLinks(const QString &s)
{
    QRegExp rx(ExternalLinkRegexpPattern);
    int ind = rx.indexIn(s);
    SkipList skip;
    while (ind >= 0) {
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

static void toHtml(QString &text, int start, int len, const QString &/*boardName*/, quint64 /*threadNumber*/,
                   bool /*processCode*/)
{
    if (start < 0 || len <= 0)
        return;
    text.replace(start, len, BTextTools::toHtml(text.mid(start, len), false));
}

static void processPostText(QString &text, const SkipList &skip, const QString &boardName, quint64 threadNumber,
                            bool processCode, ProcessPostTextFunction next)
{
    if (!next)
        return;
    if (!skip.isEmpty()) {
        const SkipPair &p = skip.last();
        int start = p.first + p.second;
        next(text, start, text.length() - start, boardName, threadNumber, processCode);
    }
    foreach (int i, bRangeR(skip.size() - 1, 0)) {
        const SkipPair &p = skip.at(i);
        int start = 0;
        if (i > 0) {
            const SkipPair &pp = skip.at(i - 1);
            start = pp.first + pp.second;
        }
        next(text, start, p.first - start, boardName, threadNumber, processCode);
    }
    if (skip.isEmpty())
        next(text, 0, text.length(), boardName, threadNumber, processCode);
}

static void processAsimmetric(QString &text, int start, int len, const QString &boardName, quint64 threadNumber,
                              bool processCode, const QString &bbTag, const QString &htmlTag,
                              ProcessPostTextFunction next, const QString &openTag = QString(), bool quote = false)
{
    if (start < 0 || len <= 0)
        return;
    SkipList skip;
    QString t = text.mid(start, len);
    QString bbop = "[" + bbTag + "]";
    QString bbcl = "[/" + bbTag + "]";
    QString op = !openTag.isEmpty() ? openTag : ("<" + htmlTag + ">");
    int s = t.indexOf(bbop, 0, Qt::CaseInsensitive);
    if (s < 0) {
        processPostText(t, skip, boardName, threadNumber, processCode, next);
        text.replace(start, len, t);
        return;
    }
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
            if (s < 0) {
                processPostText(t, skip, boardName, threadNumber, processCode, next);
                text.replace(start, len, t);
                return;
            } else {
                depth = 1;
                ind = t.indexOf(rx, s + bbop.length());
            }
        } else {
            ind = t.indexOf(rx, ind + rx.matchedLength());
        }
    }
    processPostText(t, skip, boardName, threadNumber, processCode, next);
    text.replace(start, len, t);
}

static void processSimmetric(QString &text, int start, int len, const QString &boardName, quint64 threadNumber,
                             bool processCode, const QString &wmTag1, const QString &wmTag2, const QString &htmlTag,
                             ProcessPostTextFunction next, const QString &openTag = QString(), bool quote = false)
{
    if (start < 0 || len <= 0)
        return;
    SkipList skip;
    QString t = text.mid(start, len);
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
            } else {
                s = i;
                last = t.at(i);
            }
            i += wmTag1.length();
        } else {
            ++i;
        }
    }
    processPostText(t, skip, boardName, threadNumber, processCode, next);
    text.replace(start, len, t);
}

static void processWakabaMarkExternalLink(QString &text, int start, int len, const QString &boardName,
                                          quint64 threadNumber, bool processCode)
{
    if (start < 0 || len <= 0)
        return;
    SkipList skip;
    QString t = text.mid(start, len);
    QRegExp rx(ExternalLinkRegexpPattern);
    int ind = rx.indexIn(t);
    while (ind >= 0) {
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
    processPostText(t, skip, boardName, threadNumber, processCode, &toHtml);
    text.replace(start, len, t);
}

static void processWakabaMarkLink(QString &text, int start, int len, const QString &boardName, quint64 threadNumber,
                                  bool processCode)
{
    if (start < 0 || len <= 0)
        return;
    SkipList skip;
    QString t = text.mid(start, len);
    QRegExp rx(">>\\d+");
    int ind = rx.indexIn(t);
    while (ind >= 0) {
        QString cap = rx.cap();
        QString postNumber = cap.mid(2);
        QString param = "this, '" + boardName + "', " + postNumber + ", " + QString::number(threadNumber);
        QString a = "<a href=\"javascript:selectPost(" + postNumber + ", " + QString::number(threadNumber)
                + ");\" onmouseover=\"viewPost(" + param + ");\" onmouseout=\"noViewPost();\">"
                + cap.replace(">", "&gt;") + "</a>";
        t.replace(ind, rx.matchedLength(), a);
        skip << qMakePair(ind, a.length());
        ind = rx.indexIn(t, ind + a.length());
    }
    processPostText(t, skip, boardName, threadNumber, processCode, &processWakabaMarkExternalLink);
    text.replace(start, len, t);
}

static void processWakabaMarkStrikeout(QString &text, int start, int len, const QString &boardName,
                                       quint64 threadNumber, bool processCode)
{
    if (start < 0 || len <= 0)
        return;
    SkipList skip;
    QString t = text.mid(start, len);
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
    processPostText(t, skip, boardName, threadNumber, processCode, &processWakabaMarkLink);
    text.replace(start, len, t);
}

static void processWakabaMarkItalic(QString &text, int start, int len, const QString &boardName, quint64 threadNumber,
                                    bool processCode)
{
    processSimmetric(text, start, len, boardName, threadNumber, processCode, "*", "_", "em",
                     &processWakabaMarkStrikeout);
}

static void processWakabaMarkBold(QString &text, int start, int len, const QString &boardName, quint64 threadNumber,
                                  bool processCode)
{
    processSimmetric(text, start, len, boardName, threadNumber, processCode, "**", "__", "strong",
                     &processWakabaMarkItalic);
}

static void processWakabaMarkUnderlined(QString &text, int start, int len, const QString &boardName,
                                        quint64 threadNumber, bool processCode)
{
    processSimmetric(text, start, len, boardName, threadNumber, processCode, "***", "___", "u",
                     &processWakabaMarkBold);
}

static void processWakabaMarkSpoiler(QString &text, int start, int len, const QString &boardName, quint64 threadNumber,
                                     bool processCode)
{
    processSimmetric(text, start, len, boardName, threadNumber, processCode, "%%", "", "span",
                     &processWakabaMarkUnderlined, "<span class=\"spoiler\">");
}

static void processWakabaMarkQuote(QString &text, int start, int len, const QString &boardName, quint64 threadNumber,
                                   bool processCode)
{
    if (start < 0 || len <= 0)
        return;
    SkipList skip;
    QString t = text.mid(start, len);
    QRegExp rx(">[^\n]+");
    int ind = rx.indexIn(t);
    while (ind >= 0) {
        if (!ind || t.at(ind - 1) == '\n') {
            t.insert(ind + rx.matchedLength(), "</font>");
            t.insert(ind, "<font color=\"green\">");
            skip << qMakePair(ind, 20);
            skip << qMakePair(ind + rx.matchedLength() + 20, 7);
            ind = rx.indexIn(t, ind + rx.matchedLength() + 27);
        } else {
            ind = rx.indexIn(t, ind + rx.matchedLength());
        }
    }
    processPostText(t, skip, boardName, threadNumber, processCode, &processWakabaMarkSpoiler);
    text.replace(start, len, t);
}

static void processWakabaMarkList(QString &text, int start, int len, const QString &boardName, quint64 threadNumber,
                                  bool processCode)
{
    typedef QMap<int, QString> ListTypeMap;
    init_once(ListTypeMap, listTypes, ListTypeMap()) {
        listTypes.insert(1, "disc");
        listTypes.insert(2, "circle");
        listTypes.insert(3, "square");
    }
    if (start < 0 || len <= 0)
        return;
    SkipList skip;
    QString t = text.mid(start, len);
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
    processPostText(t, skip, boardName, threadNumber, processCode, &processWakabaMarkQuote);
    text.replace(start, len, t);
}

static void processTags(QString &text, int start, int len, const QString &boardName, quint64 threadNumber,
                        bool processCode)
{
    typedef QMap<QString, QString> StringMap;
    init_once(StringMap, tags, StringMap()) {
        tags.insert("[b]", "[/b]");
        tags.insert("[i]", "[/i]");
        tags.insert("[s]", "[/s]");
        tags.insert("[u]", "[/u]");
        tags.insert("[spoiler]", "[/spoiler]");
    }
    typedef QPair<QString, QString> StringPair;
    typedef QMap<QString, StringPair> StringPairMap;
    init_once(StringPairMap, htmls, StringPairMap()) {
        htmls.insert("[b]", qMakePair(QString("<strong>"), QString("</strong>")));
        htmls.insert("[i]", qMakePair(QString("<em>"), QString("</em>")));
        htmls.insert("[s]", qMakePair(QString("<s>"), QString("</s>")));
        htmls.insert("[u]", qMakePair(QString("<u>"), QString("</u>")));
        htmls.insert("[spoiler]", qMakePair(QString("<span class=\"spoiler\">"), QString("</span>")));
    }
    ProcessPostTextFunction next = &processWakabaMarkList;
    if (start < 0 || len <= 0)
        return;
    SkipList skip;
    QString t = text.mid(start, len);
    QRegExp rxop(QStringList(tags.keys()).join("|").replace("[", "\\[").replace("]", "\\]"), Qt::CaseInsensitive);
    int s = t.indexOf(rxop);
    if (s < 0) {
        processPostText(t, skip, boardName, threadNumber, processCode, next);
        text.replace(start, len, t);
        return;
    }
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
    processPostText(t, skip, boardName, threadNumber, processCode, next);
    text.replace(start, len, t);
}

static void processTagCode(QString &text, int start, int len, const QString &boardName, quint64 threadNumber,
                           bool processCode)
{
    if (start < 0 || len <= 0)
        return;
    SkipList skip;
    QString t = text.mid(start, len);
    QString srchighlightPath = BDirTools::findResource("srchilite", BDirTools::AllResources);
    if (!srchighlightPath.isEmpty()) {
        QRegExp rx("\\[code\\s+lang\\=\"(" + Tools::supportedCodeLanguages().join("|").replace("+", "\\+")
                   + ")\"\\s*\\]");
        int indStart = rx.indexIn(t);
        int indEnd = t.indexOf("[/code]", indStart + rx.matchedLength());
        while (indStart >= 0 && indEnd > 0) {
            QString lang = rx.cap();
            lang.remove(QRegExp("\\[code\\s+lang\\=\""));
            lang.remove(QRegExp("\"\\s*\\]"));
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
                qDebug() << e.what();
                return;
            } catch (const srchilite::IOException &e) {
                qDebug() << e.what();
                return;
            } catch (const std::exception &e) {
                qDebug() << e.what();
                return;
            }
            QString result = "<div class=\"codeBlock\">" + Tools::fromStd(out.str()) + "</div>";
            t.replace(indStart, rx.matchedLength() + code.length() + 7, result);
            skip << qMakePair(indStart, result.length());
            indStart = rx.indexIn(t, indStart + result.length());
            indEnd = t.indexOf("[/code]", indStart + rx.matchedLength());
        }
    }
    processPostText(t, skip, boardName, threadNumber, processCode, &processTags);
    text.replace(start, len, t);
}

static void processTagQuote(QString &text, int start, int len, const QString &boardName, quint64 threadNumber,
                            bool processCode)
{
    ProcessPostTextFunction f = processCode ? &processTagCode : &processTags;
    processAsimmetric(text, start, len, boardName, threadNumber, processCode, "q", "font", f,
                      "<font face=\"monospace\">", true);
}

static void processWakabaMarkMonospaceSingle(QString &text, int start, int len, const QString &boardName,
                                             quint64 threadNumber, bool processCode)
{
    processSimmetric(text, start, len, boardName, threadNumber, processCode, "`", "", "font", &processTagQuote,
                     "<font face=\"monospace\">", true);
}

static void processWakabaMarkMonospaceDouble(QString &text, int start, int len, const QString &boardName,
                                             quint64 threadNumber, bool processCode)
{
    processSimmetric(text, start, len, boardName, threadNumber, processCode, "``", "", "font",
                     &processWakabaMarkMonospaceSingle, "<font face=\"monospace\">", true);
}

static std::string processPostText(QString text, const QString &boardName, quint64 threadNumber, bool processCode)
{
    processWakabaMarkMonospaceDouble(text, 0, text.length(), boardName, threadNumber, processCode);
    return Tools::toStd(text);
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
    QRegExp rx(ExternalLinkRegexpPattern);
    int ind = rx.indexIn(*s);
    while (ind >= 0) {
        s->insert(ind + rx.matchedLength(), "</a>");
        QString cap = rx.cap();
        if (!cap.startsWith("http"))
            cap += "http://";
        s->insert(ind, "<a href=\"" + cap + "\">");
        skip << qMakePair(ind, 11 + cap.length() + rx.matchedLength() + 4);
        ind = rx.indexIn(*s, ind + rx.matchedLength() + cap.length() + 15);
    }
    processPostText(*s, skip, "", 0L, false, &toHtml);
}

Content::BaseBoard::Post getPost(const cppcms::http::request &req, const QString &boardName, quint64 postNumber,
                                 quint64 threadNumber, bool *ok, QString *error)
{
    AbstractBoard *board = AbstractBoard::board(boardName);
    TranslatorQt tq(req);
    if (!board) {
        return bRet(ok, false, error, tq.translate("getPost", "Invalid board name", "error"),
                    Content::BaseBoard::Post());
    }
    bool b = false;
    Post post = Database::getPost(req, boardName, postNumber, &b, error);
    if (!b)
        return bRet(ok, false, Content::BaseBoard::Post());

    Content::BaseBoard::Post p = toController(post, board, threadNumber, tq.locale(), req);
    if (!p.number) {
        return bRet(ok, false, error, tq.translate("getPost", "Internal logic error", "error"),
                    Content::BaseBoard::Post());
    }
    return bRet(ok, true, error, QString(), p);
}

Content::BaseBoard::Post toController(const Post &post, const AbstractBoard *board, quint64 threadNumber,
                                      const QLocale &l, const cppcms::http::request &req)
{
    if (!board)
        return Content::BaseBoard::Post();
    QString storagePath = Tools::storagePath();
    if (storagePath.isEmpty())
        return Content::BaseBoard::Post();
    Content::BaseBoard::Post *p = Cache::post(board->name(), post.number());
    bool inCache = p;
    if (!p) {
        p = new Content::BaseBoard::Post;
        p->bannedFor = post.bannedFor();
        p->email = Tools::toStd(post.email());
        p->number = post.number();
        p->subject = Tools::toStd(post.subject());
        p->subjectRaw = Tools::toStd(post.subject());
        p->subjectAlwaysRaw = false;
        foreach (const QString &fn, post.files()) {
            QFileInfo fi(fn);
            Content::BaseBoard::File f;
            f.sourceName = Tools::toStd(fi.fileName());
            QString sz;
            f.thumbName = Tools::toStd(board->thumbFileName(fi.fileName(), sz, f.sizeX, f.sizeY));
            f.size = Tools::toStd(sz);
            p->files.push_back(f);
        }
        p->text = processPostText(post.text(), board->name(), threadNumber, board->processCode());
        p->showRegistered = false;
        p->showTripcode = post.showTripcode();
        if (board->showWhois()) {
            QString countryCode = Tools::countryCode(post.posterIp());
            p->flagName = Tools::toStd(Tools::flagName(countryCode));
            if (!p->flagName.empty()) {
                p->countryName = Tools::toStd(Tools::countryName(countryCode));
                if (SettingsLocker()->value("Board/guess_city_name", true).toBool())
                    p->cityName = Tools::toStd(Tools::cityName(post.posterIp()));
            } else {
                p->flagName = "default.png";
                p->countryName = "Unknown country";
            }
        }
        p->hidden = (Tools::cookieValue(req, "postHidden" + board->name() + QString::number(post.number())) == "true");
        p->ip = Tools::toStd(post.posterIp());
    }
    Content::BaseBoard::Post pp = *p;
    if (!inCache && !Cache::cachePost(board->name(), post.number(), p))
        delete p;
    TranslatorQt tq(l);
    TranslatorStd ts(l);
    for (std::list<Content::BaseBoard::File>::iterator i = pp.files.begin(); i != pp.files.end(); ++i)
        i->size = Tools::toStd(Tools::fromStd(i->size).replace("KB", tq.translate("toController", "KB", "fileSize")));
    if (board->showWhois() && "Unknown country" == pp.countryName)
        pp.countryName = ts.translate("toController", "Unknown country", "countryName");
    if (Database::registeredUserLevel(req) >= RegisteredUser::ModerLevel)
        pp.rawPostText = Tools::toStd(post.text());
    pp.dateTime = Tools::toStd(l.toString(Tools::dateTime(post.dateTime(), req), "dd/MM/yyyy ddd hh:mm:ss"));
    pp.name = Tools::toStd(toHtml(post.name()));
    if (pp.name.empty())
        pp.name = "<span class=\"userName\">" + Tools::toStd(toHtml(board->defaultUserName(l))) + "</span>";
    else
        pp.name = "<span class=\"userName\">" + pp.name + "</span>";
    pp.nameRaw = Tools::toStd(post.name());
    if (pp.nameRaw.empty())
        pp.nameRaw = Tools::toStd(board->defaultUserName(l));
    QByteArray hashpass = post.hashpass();
    if (!hashpass.isEmpty()) {
        int lvl = Database::registeredUserLevel(hashpass);
        QString name;
        pp.showRegistered = lvl >= RegisteredUser::UserLevel;
        if (!post.name().isEmpty()) {
            if (lvl >= RegisteredUser::AdminLevel)
                name = post.name();
            else if (lvl >= RegisteredUser::ModerLevel)
                name = "<span class=\"moderName\">" + toHtml(post.name()) + "</span>";
            else if (lvl >= RegisteredUser::UserLevel)
                name = "<span class=\"userName\">" + toHtml(post.name()) + "</span>";
        }
        pp.name = Tools::toStd(name);
        if (pp.name.empty())
            pp.name = "<span class=\"userName\">" + Tools::toStd(toHtml(board->defaultUserName(l))) + "</span>";
        QString s;
        hashpass += SettingsLocker()->value("Site/tripcode_salt").toString().toUtf8();
        QByteArray tripcode = QCryptographicHash::hash(hashpass, QCryptographicHash::Md5);
        foreach (int i, bRangeD(0, tripcode.size() - 1)) {
            QChar c(tripcode.at(i));
            if (c.isLetterOrNumber() || c.isPunct())
                s += c;
            else
                s += QString::number(uchar(tripcode.at(i)), 16);
        }
        pp.tripcode = Tools::toStd(s);
    }
    return pp;
}

}
