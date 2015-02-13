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

#include <QByteArray>
#include <QBuffer>
#include <QCryptographicHash>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QImage>
#include <QLocale>
#include <QMap>
#include <QMutex>
#include <QRegExp>
#include <QSettings>
#include <QString>
#include <QStringList>
#include <QVariant>

#include <cppcms/http_cookie.h>
#include <cppcms/http_file.h>
#include <cppcms/http_request.h>
#include <cppcms/json.h>

#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>

#include <istream>
#include <list>
#include <locale>
#include <ostream>
#include <streambuf>
#include <string>

namespace Tools
{

struct IpRange
{
    unsigned int start;
    unsigned int end;
public:
    bool operator <(const IpRange &other) const
    {
        return start < other.start;
    }
};

struct membuf : std::streambuf
{
public:
    explicit membuf(char *begin, char *end)
    {
        this->setg(begin, begin, end);
    }
};

static QMutex countryCodeMutex(QMutex::Recursive);
static QMutex countryNameMutex(QMutex::Recursive);
static QMutex storagePathMutex(QMutex::Recursive);

bool captchaEnabled(const QString &boardName)
{
    SettingsLocker s;
    return s->value("Board/captcha_enabled", true).toBool()
            && (boardName.isEmpty() || s->value("Board/" + boardName + "/captcha_enabled", true).toBool());
}

QString countryCode(const QString &ip)
{
    typedef QMap<IpRange, QString> CodeMap;
    countryCodeMutex.lock();
    init_once(CodeMap, codes, CodeMap()) {
        QString fn = BDirTools::findResource("res/ip_country_code_map.txt");
        QStringList sl = BDirTools::readTextFile(fn, "UTF-8").split(QRegExp("\\r?\\n+"), QString::SkipEmptyParts);
        foreach (const QString &s, sl) {
            QStringList sll = s.split(' ');
            if (sll.size() != 3)
                continue;
            IpRange r;
            bool ok = false;
            r.start = sll.first().toUInt(&ok);
            if (!ok)
                continue;
            r.end = sll.at(1).toUInt(&ok);
            if (!ok)
                continue;
            QString c = sll.last();
            if (c.length() != 2)
                continue;
            codes.insert(r, c);
        }
    }
    countryCodeMutex.unlock();
    if (ip.isEmpty())
        return "";
    QStringList sl = ip.split('.');
    if (sl.size() != 4)
        return "";
    bool ok = false;
    unsigned int n = sl.last().toUInt(&ok);
    if (!ok)
        return "";
    n += 256 * sl.at(2).toUInt(&ok);
    if (!ok)
        return "";
    n += 256 * 256 * sl.at(1).toUInt(&ok);
    if (!ok)
        return "";
    n += 256 * 256 * 256 * sl.first().toUInt(&ok);
    if (!ok || !n)
        return "";
    foreach (const IpRange &r, codes.keys()) {
        if (n >= r.start && n <= r.end)
            return codes.value(r);
    }
    return "";
}

QString countryCode(const cppcms::http::request &req)
{
    return countryCode(fromStd(const_cast<cppcms::http::request *>(&req)->remote_addr()));
}

QString countryName(const QString &countryCode)
{
    typedef QMap<QString, QString> NameMap;
    countryNameMutex.lock();
    init_once(NameMap, names, NameMap()) {
        QString fn = BDirTools::findResource("res/country_code_name_map.txt");
        QStringList sl = BDirTools::readTextFile(fn, "UTF-8").split(QRegExp("\\r?\\n+"), QString::SkipEmptyParts);
        foreach (const QString &s, sl) {
            QStringList sll = s.split(' ');
            if (sll.size() != 2)
                continue;
            if (sll.first().length() != 2 || sll.last().isEmpty())
                continue;
            names.insert(sll.first(), sll.last());
        }
    }
    countryNameMutex.unlock();
    if (countryCode.length() != 2)
        return "";
    return names.value(countryCode);
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

bool isCaptchaValid(const QString &captcha)
{
    if (captcha.isEmpty())
        return false;
    try {
        curlpp::Cleanup curlppCleanup;
        Q_UNUSED(curlppCleanup)
        QString url = "https://www.google.com/recaptcha/api/siteverify?secret=%1&response=%2";
        url = url.arg(SettingsLocker()->value("Site/captcha_private_key").toString()).arg(captcha);
        curlpp::Easy request;
        request.setOpt(curlpp::options::Url(Tools::toStd(url)));
        std::ostringstream os;
        os << request;
        QString result = Tools::fromStd(os.str());
        result.remove(QRegExp(".*\"success\":\\s*\"?"));
        result.remove(QRegExp("\"?\\,?\\s+.+"));
        return !result.compare("true", Qt::CaseInsensitive);
    } catch (curlpp::RuntimeError &e) {
        qDebug() << e.what();
        return false;
    } catch(curlpp::LogicError &e) {
        qDebug() << e.what();
        return false;
    }
}

QLocale locale(const cppcms::http::request &req, const QLocale &defaultLocale)
{
    cppcms::http::cookie localeCookie = const_cast<cppcms::http::request *>(&req)->cookie_by_name("locale");
    QLocale l(fromStd(localeCookie.value()));
    if (QLocale::c() == l)
        l = QLocale(countryCode(req));
    return (QLocale::c() == l) ? defaultLocale : l;
}

void log(const cppcms::application &app, const QString &what)
{
    log(const_cast<cppcms::application *>(&app)->request(), what);
}

void log(const cppcms::http::request &req, const QString &what)
{
    bLog("[" + userIp(req) + "] " + what);
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
        file.fileName = fromStd(f->filename());
        file.formFieldName = fromStd(f->name());
        file.mimeType = fromStd(f->mime());
        list << file;
    }
    return list;
}

bool postingEnabled(const QString &boardName)
{
    if (boardName.isEmpty() || !AbstractBoard::boardNames().contains(boardName))
        return false;
    SettingsLocker s;
    return s->value("Board/posting_enabled", true).toBool()
            && s->value("Board/" + boardName + "/posting_enabled", true).toBool();
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
    QByteArray jsonBa = BDirTools::readFile(fileName, -1, &b);
    if (!b)
        return cppcms::json::value();
    char *jsonData = jsonBa.data();
    membuf jsonBuff(jsonData, jsonData + jsonBa.size());
    std::istream jsonIn(&jsonBuff);
    cppcms::json::value json;
    if (json.load(jsonIn, true))
        return bRet(ok, true, json);
    else
        return bRet(ok, false, cppcms::json::value());
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

QString saveFile(const File &f, const QString &boardName, bool *ok)
{
    QString storagePath = Tools::storagePath();
    if (boardName.isEmpty() || storagePath.isEmpty())
        return bRet(ok, false, QString());
    QString path = storagePath + "/img/" + boardName;
    if (!BDirTools::mkpath(path))
        return bRet(ok, false, QString());
    QString dt = QString::number(QDateTime::currentDateTimeUtc().toMSecsSinceEpoch());
    QString suffix = QFileInfo(f.fileName).suffix();
    QImage img;
    QByteArray data = f.data;
    QBuffer buff(&data);
    buff.open(QIODevice::ReadOnly);
    if (!img.load(&buff, suffix.toLower().toLatin1().data()))
        return bRet(ok, false, QString());
    if (img.height() > 200 || img.width() > 200)
        img = img.scaled(200, 200, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    QString sfn = path + "/" + dt + "." + suffix;
    if (!BDirTools::writeFile(sfn, f.data))
        return bRet(ok, false, QString());
    if (!suffix.compare("gif", Qt::CaseInsensitive))
        suffix = "png";
    if (!img.save(path + "/" + dt + "s." + suffix, suffix.toLower().toLatin1().data()))
        return bRet(ok, false, QString());
    return bRet(ok, true, sfn);
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

Post toPost(const PostParameters &params, const FileList &files)
{
    Post p;
    p.captcha = params.value("g-recaptcha-response");
    p.email = params.value("email");
    p.files = files;
    p.name = params.value("name");
    QString pwd = params.value("password");
    if (pwd.isEmpty())
        pwd = SettingsLocker()->value("Board/default_post_password").toString();
    p.password = QCryptographicHash::hash(pwd.toLocal8Bit(), QCryptographicHash::Sha1);;
    p.subject = params.value("subject");
    p.text = params.value("text");
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

QString userIp(const cppcms::http::request &req)
{
    if (SettingsLocker()->value("System/use_x_real_ip", false).toBool())
        return fromStd(const_cast<cppcms::http::request *>(&req)->getenv("HTTP_X_REAL_IP"));
    else
        return fromStd(const_cast<cppcms::http::request *>(&req)->remote_addr());
}

}
