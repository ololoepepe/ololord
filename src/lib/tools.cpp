#include "tools.h"

#include "board/abstractboard.h"
#include "cache.h"
#include "controller/ban.h"
#include "controller/board.h"
#include "controller/error.h"
#include "controller/notfound.h"
#include "settingslocker.h"
#include "translator.h"

#include <BCoreApplication>
#include <BDirTools>
#include <BLogger>
#include <BTextTools>

#include <QByteArray>
#include <QBuffer>
#include <QCryptographicHash>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QImage>
#include <QList>
#include <QLocale>
#include <QMap>
#include <QMutex>
#include <QPair>
#include <QRegExp>
#include <QSet>
#include <QSettings>
#include <QString>
#include <QStringList>
#include <QTime>
#include <QVariant>

#include <cppcms/http_cookie.h>
#include <cppcms/http_file.h>
#include <cppcms/http_request.h>
#include <cppcms/json.h>

#include <magic.h>

#include <cmath>
#include <istream>
#include <list>
#include <locale>
#include <ostream>
#include <streambuf>
#include <string>

namespace Tools
{

/*static QString ipStr(unsigned int ip)
{
    QString s;
    s += QString::number(ip / (256 * 256 * 256)) + ".";
    ip %= (256 * 256 * 256);
    s += QString::number(ip / (256 * 256)) + ".";
    ip %= (256 * 256);
    s += QString::number(ip / 256) + ".";
    s += QString::number(ip % 256);
    return s;
}*/

IpRange::IpRange(const QString &text, const QChar &separator)
{
    start = 0;
    end = 0;
    QStringList sl = text.split(separator);
    if (sl.size() > 2)
        return;
    bool ok = false;
    start = Tools::ipNum(sl.first(), &ok);
    if (!ok)
        return;
    if (sl.size() == 2) {
        end = Tools::ipNum(sl.at(1), &ok);
        if (!ok) {
            start = 0;
            return;
        }
    } else {
        end = start;
    }
}

IpRange::IpRange(const QStringList &sl, int startIndex, int endIndex, bool num)
{
    start = 0;
    end = 0;
    if (startIndex < 0 || endIndex < 0 || startIndex >= sl.size() || endIndex >= sl.size())
        return;
    bool ok = false;
    if (num) {
        start = sl.at(startIndex).toUInt(&ok);
        if (!ok)
            return;
        end = sl.at(endIndex).toUInt(&ok);
    } else {
        start = Tools::ipNum(sl.at(startIndex), &ok);
        if (!ok)
            return;
        end = Tools::ipNum(sl.at(endIndex), &ok);
    }
    if (!ok) {
        start = 0;
        return;
    }
}

void IpRange::clear()
{
    start = 0;
    end = 0;
}

bool IpRange::in(unsigned int ip) const
{
    if (!ip || !isValid())
        return false;
    return ip >= start && ip <= end;
}

bool IpRange::in(const QString &ip) const
{
    return in(Tools::ipNum(ip));
}

bool IpRange::isValid() const
{
    return start && end;
}

bool IpRange::operator <(const IpRange &other) const
{
    return start < other.start;
}

IpBanInfo::IpBanInfo(const QStringList &sl) :
    range(!sl.isEmpty() ? sl.first() : QString())
{
    if (sl.size() != 2) {
        range.clear();
        return;
    }
    bool ok = false;
    level = sl.at(1).toInt(&ok);
    if (!ok)
        range.clear();
}

bool IpBanInfo::isValid() const
{
    return range.isValid();
}

static QMutex cityNameMutex(QMutex::Recursive);
static QMutex countryCodeMutex(QMutex::Recursive);
static QMutex countryNameMutex(QMutex::Recursive);
static QList<IpRange> loggingSkipIps;
static QMutex loggingSkipIpsMutex(QMutex::Recursive);
static QMutex storagePathMutex(QMutex::Recursive);
static QMutex timezoneMutex(QMutex::Recursive);

static QTime time(int msecs)
{
    int h = msecs / BeQt::Hour;
    msecs %= BeQt::Hour;
    int m = msecs / BeQt::Minute;
    msecs %= BeQt::Minute;
    int s = msecs / BeQt::Second;
    return QTime(h, m, s, msecs % BeQt::Second);
}

QStringList acceptedExternalBoards()
{
    QString fn = BDirTools::findResource("res/echo.txt", BDirTools::UserOnly);
    return BDirTools::readTextFile(fn, "UTF-8").split(QRegExp("\\r?\\n+"), QString::SkipEmptyParts);
}

bool captchaEnabled(const QString &boardName)
{
    SettingsLocker s;
    return s->value("Board/captcha_enabled", true).toBool()
            && (boardName.isEmpty() || s->value("Board/" + boardName + "/captcha_enabled", true).toBool());
}

QString cityName(const QString &ip)
{
    typedef QMap<IpRange, QString> NameMap;
    QMutexLocker locker(&cityNameMutex);
    init_once(NameMap, names, NameMap()) {
        QString fn = BDirTools::findResource("res/ip_city_name_map.txt");
        QStringList sl = BDirTools::readTextFile(fn, "UTF-8").split(QRegExp("\\r?\\n+"), QString::SkipEmptyParts);
        foreach (const QString &s, sl) {
            QStringList sll = s.split(' ');
            if (sll.size() < 3)
                continue;
            IpRange r(sll, 0, 1, true);
            if (!r.isValid())
                continue;
            QString n = QStringList(sll.mid(2)).join(" ");
            if (n.length() < sll.size() - 2)
                continue;
            names.insert(r, n);
        }
    }
    unsigned int n = ipNum(ip);
    if (!n)
        return "";
    static QMap<unsigned int, QString> map;
    QString code = map.value(n);
    if (!code.isEmpty())
        return ("-" != code) ? code : "";
    foreach (const IpRange &r, names.keys()) {
        if (n >= r.start && n <= r.end) {
            QString name = names.value(r);
            map.insert(n, name);
            return name;
        }
    }
    map.insert(n, "-");
    return "";
}

QString cityName(const cppcms::http::request &req)
{
    return cityName(fromStd(const_cast<cppcms::http::request *>(&req)->remote_addr()));
}

QString cookieValue(const cppcms::http::request &req, const QString &name)
{
    if (name.isEmpty())
        return "";
    return fromStd(const_cast<cppcms::http::request *>(&req)->cookie_by_name(toStd(name)).value());
}

QString countryCode(const QString &ip)
{
    typedef QMap<IpRange, QString> CodeMap;
    QMutexLocker locker(&countryCodeMutex);
    init_once(CodeMap, codes, CodeMap()) {
        QString fn = BDirTools::findResource("res/ip_country_code_map.txt");
        QStringList sl = BDirTools::readTextFile(fn, "UTF-8").split(QRegExp("\\r?\\n+"), QString::SkipEmptyParts);
        foreach (const QString &s, sl) {
            QStringList sll = s.split(' ');
            if (sll.size() != 3)
                continue;
            IpRange r(sll, 0, 1, true);
            if (!r.isValid())
                continue;
            QString c = sll.last();
            if (c.length() != 2)
                continue;
            codes.insert(r, c);
        }
    }
    unsigned int n = ipNum(ip);
    if (!n)
        return "";
    static QMap<unsigned int, QString> map;
    QString code = map.value(n);
    if (!code.isEmpty())
        return ("-" != code) ? code : "";
    foreach (const IpRange &r, codes.keys()) {
        if (n >= r.start && n <= r.end) {
            QString code = codes.value(r);
            map.insert(n, code);
            return code;
        }
    }
    map.insert(n, "-");
    return "";
}

QString countryCode(const cppcms::http::request &req)
{
    return countryCode(fromStd(const_cast<cppcms::http::request *>(&req)->remote_addr()));
}

QString countryName(const QString &countryCode)
{
    typedef QMap<QString, QString> NameMap;
    QMutexLocker locker(&countryNameMutex);
    init_once(NameMap, names, NameMap()) {
        QString fn = BDirTools::findResource("res/country_code_name_map.txt");
        QStringList sl = BDirTools::readTextFile(fn, "UTF-8").split(QRegExp("\\r?\\n+"), QString::SkipEmptyParts);
        foreach (const QString &s, sl) {
            QStringList sll = s.split(' ');
            if (sll.size() < 2)
                continue;
            if (sll.first().length() != 2)
                continue;
            QString name = QStringList(sll.mid(1)).join(" ");
            if (name.length() < sll.length() - 1)
                continue;
            names.insert(sll.first(), name);
        }
    }
    if (countryCode.length() != 2)
        return "";
    return names.value(countryCode);
}

QString customHomePageContent(const QLocale &l)
{
    QString *s = Cache::customHomePageContent(l);
    if (!s) {
        QString path = BDirTools::findResource("homepage", BDirTools::UserOnly);
        if (path.isEmpty())
            return QString();
        QString fn = BDirTools::localeBasedFileName(path + "/content.html", l);
        if (fn.isEmpty())
            return QString();
        s = new QString(BDirTools::readTextFile(fn, "UTF-8"));
        if (!Cache::cacheCustomHomePageContent(l, s)) {
            QString ss = *s;
            delete s;
            return ss;
        }
    }
    return *s;
}

QDateTime dateTime(const QDateTime &dt, const cppcms::http::request &req)
{
    QString s = cookieValue(req, "time");
    if (s.isEmpty() || s.compare("local", Qt::CaseInsensitive))
        return localDateTime(dt);
    return localDateTime(dt, timeZoneMinutesOffset(req));
}

QString externalLinkRegexpPattern()
{
    init_once(QString, pattern, QString()) {
        QString fn = BDirTools::findResource("res/root-zones.txt", BDirTools::GlobalOnly);
        QString s = BDirTools::readTextFile(fn, "UTF-8").split(QRegExp("\\r?\\n+"), QString::SkipEmptyParts).join("|");
        if (s.isEmpty())
            s = "[a-z]{2,6}\\.?";
        pattern = "(https?:\\/\\/)?([\\w\\.\\-]+)\\.(" + s + ")(\\/[\\w\\.\\-\\?\\=#~&%]*)*\\/?(?!\\S)";
    }
    return pattern;
}

QString flagName(const QString &countryCode)
{
    if (countryCode.length() != 2)
        return "";
    QString fn = BDirTools::findResource("static/img/flag/" + countryCode.toUpper() + ".png");
    return !fn.isEmpty() ? QFileInfo(fn).fileName() : QString();
}

QLocale fromStd(const std::locale &l)
{
    return QLocale(fromStd(l.name()).split('.').first());
}

QString fromStd(const std::string &s)
{
    return QString::fromLocal8Bit(s.data());
}

QStringList fromStd(const std::list<std::string> &sl)
{
    QStringList list;
    foreach (const std::string &s, sl)
        list << fromStd(s);
    return list;
}

GetParameters getParameters(const cppcms::http::request &request)
{
    PostParameters m;
    cppcms::http::request::form_type data = const_cast<cppcms::http::request *>(&request)->get();
    for (cppcms::http::request::form_type::iterator i = data.begin(); i != data.end(); ++i)
        m.insert(fromStd(i->first), fromStd(i->second));
    return m;
}

QByteArray hashpass(const cppcms::http::request &req)
{
    return toHashpass(hashpassString(req));
}

QString hashpassString(const cppcms::http::request &req)
{
    return cookieValue(req, "hashpass");
}

int ipBanLevel(const QString &ip)
{
    bool ok = false;
    int n = ipNum(ip, &ok);
    if (!ok)
        return 0;
    Cache::IpBanInfoList *list = Cache::ipBanInfoList();
    int level = 0;
    if (!list) {
        QString path = BDirTools::findResource("res/ip_ban.txt", BDirTools::UserOnly);
        if (path.isEmpty())
            return 0;
        QStringList sl = BDirTools::readTextFile(path, "UTF-8").split(QRegExp("\\r?\\n+"), QString::SkipEmptyParts);
        list = new Cache::IpBanInfoList;
        foreach (const QString &s, sl) {
            IpBanInfo inf(s.split(' '));
            if (!inf.isValid())
                continue;
            if (inf.range.in(n))
                level = inf.level;
            *list << inf;
        }
        if (!Cache::cacheIpBanInfoList(list))
            delete list;
    } else {
        foreach (const IpBanInfo &inf, *list) {
            if (inf.range.in(n)) {
                level = inf.level;
                break;
            }
        }
    }
    return level;
}

int ipBanLevel(const cppcms::http::request &req)
{
    return ipBanLevel(userIp(req));
}

bool isAudioType(const QString &mimeType)
{
    typedef QSet<QString> StringSet;
    init_once(StringSet, types, StringSet()) {
        types.insert("audio/mpeg");
        types.insert("audio/ogg");
        types.insert("audio/wav");
    }
    return types.contains(mimeType);
}

bool isImageType(const QString &mimeType)
{
    typedef QSet<QString> StringSet;
    init_once(StringSet, types, StringSet()) {
        types.insert("image/gif");
        types.insert("image/jpeg");
        types.insert("image/png");
    }
    return types.contains(mimeType);
}

unsigned int ipNum(const QString &ip, bool *ok)
{
    QStringList sl = ip.split('.');
    if (sl.size() != 4)
        return bRet(ok, false, 0);
    bool b = false;
    unsigned int n = sl.last().toUInt(&b);
    if (!b)
        return bRet(ok, false, 0);
    n += 256 * sl.at(2).toUInt(&b);
    if (!b)
        return bRet(ok, false, 0);
    n += 256 * 256 * sl.at(1).toUInt(&b);
    if (!b)
        return bRet(ok, false, 0);
    n += 256 * 256 * 256 * sl.first().toUInt(&b);
    if (!b || !n)
        return bRet(ok, false, 0);
    return bRet(ok, true, n);
}

bool isSpecialThumbName(const QString &tn)
{
    return isAudioType(tn) || isImageType(tn) || isVideoType(tn);
}

bool isVideoType(const QString &mimeType)
{
    typedef QSet<QString> StringSet;
    init_once(StringSet, types, StringSet()) {
        types.insert("video/mp4");
        types.insert("video/ogg");
        types.insert("video/webm");
    }
    return types.contains(mimeType);
}

QDateTime localDateTime(const QDateTime &dt, int offsetMinutes)
{
    static const int MaxMsecs = 24 * BeQt::Hour;
    if (offsetMinutes < -720 || offsetMinutes > 840)
        return dt.toLocalTime();
    QDateTime ndt = dt.toUTC();
    QTime t = ndt.time();
    int msecs = t.hour() * BeQt::Hour + t.minute() * BeQt::Minute + t.second() * BeQt::Second * t.msec();
    int msecsOffset = offsetMinutes * BeQt::Minute;
    msecs += msecsOffset;
    if (msecs < 0) {
        ndt.setDate(ndt.date().addDays(-1));
        ndt.setTime(time(msecs + MaxMsecs));
    } else if (msecs >= MaxMsecs) {
        ndt.setDate(ndt.date().addDays(1));
        ndt.setTime(time(MaxMsecs - msecs));
    } else {
        ndt.setTime(time(msecs));
    }
    return ndt;
}

QLocale locale(const cppcms::http::request &req, const QLocale &defaultLocale)
{
    QLocale l(cookieValue(req, "locale"));
    if (QLocale::c() == l)
        l = QLocale(countryCode(req));
    return (QLocale::c() == l) ? defaultLocale : l;
}

void log(const cppcms::application &app, const QString &action, const QString &state, const QString &target)
{
    log(const_cast<cppcms::application *>(&app)->request(), action, state, target);
}

void log(const cppcms::http::request &req, const QString &action, const QString &state, const QString &target)
{
    do_once(init)
        resetLoggingSkipIps();
    QString ip = userIp(req);
    int n = ipNum(ip);
    QMutexLocker locker(&loggingSkipIpsMutex);
    foreach (const IpRange &r, loggingSkipIps) {
        if (r.in(n))
            return;
    }
    locker.unlock();
    bLog("[" + ip + "] [" + action + "] [" + state + "]" + (!target.isEmpty() ? (" " + target) : QString()));
}

void log(const char *where, const std::exception &e)
{
    QString s = "[" + QString::fromLatin1(where) + "] [" + QString::fromLatin1(typeid(e).name()) + "]"
            + Tools::fromStd(e.what());
    qDebug() << s;
    bLog(s);
}

void log(const char *where)
{
    QString s = "[" + QString::fromLatin1(where) + "] [UNKNOWN_EXCEPTION_TYPE]";
    qDebug() << s;
    bLog(s);
}

unsigned int maxInfo(MaxInfo m, const QString &boardName)
{
    typedef QMap< MaxInfo, QPair<QString, uint> > MaxMap;
    init_once(MaxMap, map, MaxMap()) {
        map.insert(MaxEmailFieldLength, qMakePair(QString("max_email_length"), uint(150)));
        map.insert(MaxNameFieldLength, qMakePair(QString("max_name_length"), uint(50)));
        map.insert(MaxSubjectFieldLength, qMakePair(QString("max_subject_length"), uint(150)));
        map.insert(MaxTextFieldLength, qMakePair(QString("max_text_length"), uint(15000)));
        map.insert(MaxPasswordFieldLength, qMakePair(QString("max_password_length"), uint(150)));
        map.insert(MaxFileCount, qMakePair(QString("max_file_count"), uint(1)));
        map.insert(MaxFileSize, qMakePair(QString("max_file_size"), uint(10)));
        map.insert(MaxLastPosts, qMakePair(QString("max_last_posts"), uint(3)));
    }
    if (!map.contains(m))
        return 0;
    QPair<QString, uint> p = map.value(m);
    SettingsLocker s;
    if (boardName.isEmpty())
        return s->value("Board/" + p.first, p.second).toUInt();
    else
        return s->value("Board/" + boardName + "/" + p.first, s->value("Board/" + p.first, p.second)).toUInt();
}

QString mimeType(const QByteArray &data, bool *ok)
{
    try {
        if (data.isEmpty())
            return bRet(ok, false, QString());
        qDebug() << "MIME" << data.size();
        BDirTools::writeFile("/home/darkangel/tmp/debug-mime", data);
        magic_t magicMimePredictor;
        magicMimePredictor = magic_open(MAGIC_MIME_TYPE);
        if (!magicMimePredictor)
            return bRet(ok, false, QString());
        if (magic_load(magicMimePredictor, 0)) {
            magic_close(magicMimePredictor);
            return bRet(ok, false, QString());
        }
        QString result = QString::fromLatin1(magic_buffer(magicMimePredictor, (void *) data.data(), data.size()));
        return bRet(ok, !result.isEmpty(), result);
    } catch (...) {
        return bRet(ok, false, QString());
    }
}

QStringList news(const QLocale &l)
{
    QStringList *sl = Cache::news(l);
    if (!sl) {
        QString path = BDirTools::findResource("news", BDirTools::UserOnly);
        if (path.isEmpty())
            return QStringList();
        QString fn = BDirTools::localeBasedFileName(path + "/news.txt", l);
        if (fn.isEmpty())
            return QStringList();
        sl = new QStringList(BDirTools::readTextFile(fn, "UTF-8").split(QRegExp("\\r?\\n+"), QString::SkipEmptyParts));
        if (!Cache::cacheNews(l, sl)) {
            QStringList sll = *sl;
            delete sl;
            return sll;
        }
    }
    return *sl;
}

FileList postFiles(const cppcms::http::request &request)
{
    FileList list;
    cppcms::http::request::files_type files = const_cast<cppcms::http::request *>(&request)->files();
    foreach (int i, bRangeD(0, files.size() - 1)) {
        cppcms::http::file *f = files.at(i).get();
        if (!f)
            continue;
        File file;
        std::istream &in = f->data();
        char *buff = new char[f->size()];
        in.read(buff, f->size());
        file.data = QByteArray::fromRawData(buff, f->size());
        file.fileName = QFileInfo(fromStd(f->filename())).fileName();
        file.formFieldName = fromStd(f->name());
        file.mimeType = fromStd(f->mime());
        list << file;
    }
    return list;
}

PostParameters postParameters(const cppcms::http::request &request)
{
    PostParameters m;
    cppcms::http::request::form_type data = const_cast<cppcms::http::request *>(&request)->post();
    for (cppcms::http::request::form_type::iterator i = data.begin(); i != data.end(); ++i)
        m.insert(fromStd(i->first), fromStd(i->second));
    return m;
}

cppcms::json::value readJsonValue(const QString &fileName, bool *ok)
{
    bool b = false;
    QString s = BDirTools::readTextFile(fileName, "UTF-8", &b);
    if (!b)
        return cppcms::json::value();
    cppcms::json::value json;
    std::stringstream in(toStd(s));
    if (json.load(in, true))
        return bRet(ok, true, json);
    else
        return bRet(ok, false, cppcms::json::value());
}

void resetLoggingSkipIps()
{
    QStringList list = SettingsLocker()->value("System/logging_skip_ip").toString().split(QRegExp("\\,\\s*"),
                                                                                          QString::SkipEmptyParts);
    QMutexLocker locker(&loggingSkipIpsMutex);
    loggingSkipIps.clear();
    foreach (const QString &s, list) {
        IpRange r(s);
        if (!r.isValid())
            continue;
        loggingSkipIps << r;
    }
}

QStringList rules(const QString &prefix, const QLocale &l)
{
    QStringList *sl = Cache::rules(l, prefix);
    if (!sl) {
        QString path = BDirTools::findResource(prefix, BDirTools::UserOnly);
        if (path.isEmpty())
            return QStringList();
        QString fn = BDirTools::localeBasedFileName(path + "/rules.txt", l);
        if (fn.isEmpty())
            return QStringList();
        sl = new QStringList(BDirTools::readTextFile(fn, "UTF-8").split(QRegExp("\\r?\\n+"), QString::SkipEmptyParts));
        if (!Cache::cacheRules(prefix, l, sl)) {
            QStringList sll = *sl;
            delete sl;
            return sll;
        }
    }
    return *sl;
}

FriendList siteFriends()
{
    FriendList *list = Cache::friendList();
    if (!list) {
        QString path = BDirTools::findResource("res/friends.txt", BDirTools::UserOnly);
        if (path.isEmpty())
            return FriendList();
        QStringList sl = BDirTools::readTextFile(path, "UTF-8").split(QRegExp("\\r?\\n+"), QString::SkipEmptyParts);
        list = new FriendList;
        foreach (const QString &s, sl) {
            bool ok = false;
            QStringList sll = BTextTools::splitCommand(s, &ok);
            if (!ok || sll.size() < 2 || sll.size() > 3)
                continue;
            Friend f;
            f.url = sll.first();
            f.name = sll.at(1);
            if (sll.size() > 2)
                f.title = sll.last();
            if (f.url.isEmpty() || f.name.isEmpty())
                continue;
            *list << f;
        }
        if (!Cache::cacheFriendList(list)) {
            FriendList llist = *list;
            delete list;
            return llist;
        }
    }
    return *list;
}

QString storagePath()
{
    storagePathMutex.lock();
    init_once(QString, path, QString())
        path = BDirTools::findResource("storage", BDirTools::UserOnly);
    storagePathMutex.unlock();
    return path;
}

QStringList supportedCodeLanguages()
{
    QString srchighlightPath = BDirTools::findResource("srchilite");
    if (srchighlightPath.isEmpty())
        return QStringList();
    QStringList sl = QDir(srchighlightPath).entryList(QStringList() << "*.lang", QDir::Files);
    foreach (int i, bRangeD(0, sl.size() - 1))
        sl[i].remove(".lang");
    int ind = sl.indexOf("cpp");
    if (ind >= 0)
        sl.insert(ind + 1, "c++");
    return sl;
}

int timeZoneMinutesOffset(const cppcms::http::request &req)
{
    typedef QMap<QString, int> TimezoneMap;
    QMutexLocker locker(&timezoneMutex);
    init_once(TimezoneMap, timezones, TimezoneMap()) {
        QString fn = BDirTools::findResource("res/city_name_timezone_map.txt");
        QStringList sl = BDirTools::readTextFile(fn, "UTF-8").split(QRegExp("\\r?\\n+"), QString::SkipEmptyParts);
        foreach (const QString &s, sl) {
            QStringList sll = s.split(' ');
            if (sll.size() < 2)
                continue;
            if (sll.last().isEmpty())
                continue;
            bool ok = false;
            int offset = sll.last().toInt(&ok);
            if (!ok || offset < -720 || offset > 840)
                continue;
            timezones.insert(QStringList(sll.mid(0, sll.size() - 1)).join(" "), offset);
        }
    }
    return SettingsLocker()->value("Board/guess_city_name", true).toBool() ? timezones.value(cityName(req), -1000) :
                                                                             -1000;
}

QByteArray toHashpass(const QString &s, bool *ok)
{
    if (s.length() != 44)
        return bRet(ok, false, QByteArray());
    QStringList sl = s.split('-');
    if (sl.size() != 5)
        return bRet(ok, false, QByteArray());
    QByteArray ba;
    foreach (const QString &ss, sl) {
        if (ss.length() != 8 || !QRegExp("([0-9a-fA-F]){8}").exactMatch(ss))
            return bRet(ok, false, QByteArray());
        char c[4];
        foreach (int i, bRangeD(0, 3)) {
            bool b = false;
            c[i] = ss.mid(i * 2, 2).toUShort(&b, 16);
            if (!b)
                return bRet(ok, false, QByteArray());
        }
        ba.append(c, 4);
    }
    return bRet(ok, true, ba);
}

Post toPost(const PostParameters &params, const FileList &files)
{
    Post p;
    p.email = params.value("email");
    p.fileHashes = params.value("fileHashes").split(',', QString::SkipEmptyParts);
    p.files = files;
    p.name = params.value("name");
    QString pwd = params.value("password");
    if (pwd.isEmpty())
        pwd = SettingsLocker()->value("Board/default_post_password").toString();
    p.password = QCryptographicHash::hash(pwd.toLocal8Bit(), QCryptographicHash::Sha1);
    p.raw = !params.value("raw").compare("true", Qt::CaseInsensitive);
    p.subject = params.value("subject");
    p.text = params.value("text");
    p.draft = !params.value("draft").compare("true", Qt::CaseInsensitive);
    return p;
}

Post toPost(const cppcms::http::request &req)
{
    return toPost(postParameters(req), postFiles(req));
}

std::locale toStd(const QLocale &l)
{
    return std::locale(toStd(l.name()).data());
}

std::string toStd(const QString &s)
{
    return std::string(s.toLocal8Bit().data());
}

std::list<std::string> toStd(const QStringList &sl)
{
    std::list<std::string> list;
    foreach (const QString &s, sl)
        list.push_back(toStd(s));
    return list;
}

QString toString(const QByteArray &hp, bool *ok)
{
    if (hp.size() != 20)
        return bRet(ok, false, QString());
    QString s;
    foreach (int i, bRangeD(0, hp.size() - 1)) {
        QString c = QString::number(uchar(hp.at(i)), 16);
        if (c.length() < 2)
            c.prepend("0");
        s += c;
        if ((i != hp.size() - 1) && !((i + 1) % 4))
            s += "-";
    }
    bool b = false;
    toHashpass(s, &b);
    if (!b)
        return bRet(ok, false, QString());
    return bRet(ok, true, s);
}

QString userIp(const cppcms::http::request &req, bool *proxy)
{
    SettingsLocker s;
    cppcms::http::request &r = *const_cast<cppcms::http::request *>(&req);
    bSet(proxy, false);
    if (s->value("System/Proxy/detect_real_ip", true).toBool()) {
        QString ip = fromStd(r.getenv("HTTP_X_FORWARDED_FOR"));
        bool ok = false;
        ipNum(ip, &ok);
        if (ok)
            return bRet(proxy, true, ip);
        ip = fromStd(r.getenv("HTTP_X_CLIENT_IP"));
        ipNum(ip, &ok);
        if (ok)
            return bRet(proxy, true, ip);
    }
    if (s->value("System/use_x_real_ip", false).toBool())
        return fromStd(r.getenv("HTTP_X_REAL_IP"));
    else
        return fromStd(r.remote_addr());
}

}
