#include "staticfilesroute.h"

#include "board/abstractboard.h"
#include "cache.h"
#include "controller/controller.h"
#include "settingslocker.h"
#include "tools.h"

#include <BDirTools>
#include <BeQt>
#include <BTextTools>

#include <QByteArray>
#include <QDebug>
#include <QFileInfo>
#include <QList>
#include <QMap>
#include <QPair>
#include <QRegExp>
#include <QSettings>
#include <QString>
#include <QStringList>
#include <QTextCodec>
#include <QVariant>

#include <cppcms/application.h>
#include <cppcms/http_request.h>
#include <cppcms/http_response.h>

#include <ctime>

StaticFilesRoute::StaticFilesRoute(cppcms::application &app, Mode m) :
    AbstractRoute(app), mode(m), Prefix((StaticFilesMode == m) ? "static" : "storage/img")
{
    //
}

void StaticFilesRoute::handle(std::string p)
{
    QString path = Tools::fromStd(p);
    QString logAction = QString(StaticFilesMode == mode ? "static" : "dynamic") + "_file";
    QString logTarget = path;
    Tools::log(application, logAction, "begin", logTarget);
    typedef Cache::File *(*GetCacheFunction)(const QString &path);
    typedef Cache::File *(*SetCacheFunction)(const QString &path, const QByteArray &file);
    QString err;
    if (!Controller::testRequestNonAjax(application, Controller::GetRequest, &err))
        return Tools::log(application, logAction, "fail:" + err, logTarget);
    if (path.contains("../") || path.contains("/..")) { //NOTE: Are you trying to cheat me?
        Controller::renderNotFoundNonAjax(application);
        Tools::log(application, logAction, "fail:cheating", logTarget);
        return;
    }
    GetCacheFunction getCache = (StaticFilesMode == mode) ? &Cache::staticFile : &Cache::dynamicFile;
    SetCacheFunction setCache = (StaticFilesMode == mode) ? &Cache::cacheStaticFile : &Cache::cacheDynamicFile;
    Cache::File *file = getCache(path);
    if (file) {
        write(file->data, file->msecsSinceEpoch);
        Tools::log(application, logAction, "success:cache", logTarget);
        return;
    }
    QString fn = BDirTools::findResource(Prefix + "/" + path, BDirTools::AllResources);
    //NOTE: Files stored in memory are also cached. It's OK (think of If-Modified-Since).
    bool ok = false;
    QByteArray ba = BDirTools::readFile(fn, -1, &ok);
    if (!ok) {
        Controller::renderNotFoundNonAjax(application);
        Tools::log(application, logAction, "fail:not_found", logTarget);
        return;
    }
    file = setCache(path, ba);
    if (file)
        write(file->data, file->msecsSinceEpoch);
    else
        write(ba);
    Tools::log(application, logAction, "success", logTarget);
}

void StaticFilesRoute::handle(std::string boardName, std::string path)
{
    handle(boardName + "/" + path);
}

unsigned int StaticFilesRoute::handlerArgumentCount() const
{
    return (StaticFilesMode == mode) ? 1 : 2;
}

std::string StaticFilesRoute::key() const
{
    return (StaticFilesMode == mode) ? "staticFiles" : "dynamicFiles";
}

int StaticFilesRoute::priority() const
{
    return (StaticFilesMode == mode) ? 60 : 50;
}

std::string StaticFilesRoute::regex() const
{
    static const QString boardRx = "(" + AbstractBoard::boardNames().join("|") + ")";
    return (StaticFilesMode == mode) ? "/(.+)" : Tools::toStd("/" + boardRx + "/(.+)");
}

std::string StaticFilesRoute::url() const
{
    return (StaticFilesMode == mode) ? "/{1}" : "/{1}/{2}";
}

void StaticFilesRoute::write(const QByteArray &data, qint64 msecsSinceEpoch)
{
    typedef QPair<uint, uint> Range;
    cppcms::http::response &r = application.response();
    QString s = Tools::fromStd(application.request().getenv("HTTP_IF_MODIFIED_SINCE"));
    s.remove(" GMT").remove(QRegExp("^\\S+\\s+"));
    if (msecsSinceEpoch > 0 && !s.isEmpty()) {
        QLocale l = QLocale::c();
        foreach (int i, bRangeD(1, 12)) {
            QString m = QString::number(i);
            if (m.length() < 2)
                m.prepend('0');
            s.replace(l.monthName(i, QLocale::ShortFormat), m);
        }
        QDateTime dt = QDateTime::fromString(s, "dd MM yyyy hh:mm:ss");
        if (!dt.isValid())
            dt = QDateTime::fromString(s, "dd-MM-yy hh:mm:ss");
        if (!dt.isValid())
            dt = QDateTime::fromString(s, "MM dd hh:mm:ss yyyy");
        dt.setTimeSpec(Qt::UTC);
        if (dt.isValid() && dt.toLocalTime() >= QDateTime::fromMSecsSinceEpoch(msecsSinceEpoch))
            return r.status(304);
    }
    r.content_type("");
    r.accept_ranges("bytes");
    if (msecsSinceEpoch > 0)
        r.last_modified(QDateTime::fromMSecsSinceEpoch(msecsSinceEpoch).toTime_t());
    QString range = Tools::fromStd(application.request().http_range());
    QList<Range> list;
    if (!range.isEmpty() && range.startsWith("bytes=", Qt::CaseInsensitive)) {
        foreach (const QString &s, range.mid(6).split(',')) {
            QRegExp rx1("([0-9]+)\\-([0-9]+)");
            QRegExp rx2("\\-([0-9]+)");
            QRegExp rx3("([0-9]+)\\-");
            Range p;
            if (rx1.exactMatch(s))
                p = qMakePair(rx1.cap(1).toUInt(), rx1.cap(2).toUInt());
            else if (rx2.exactMatch(s))
                p = qMakePair(uint(data.size()) - rx2.cap(1).toUInt(), uint(data.size() - 1));
            else if (rx3.exactMatch(s))
                p = qMakePair(rx3.cap(1).toUInt(), uint(data.size() - 1));
            else
                continue;
            if (p.first > p.second || int(p.second) >= data.size())
                continue;
            if (!list.isEmpty() && p.first <= list.last().second)
                continue;
            list << p;
        }
    }
    QByteArray ba;
    if (list.size() > 1) {
        r.status(206);
        static const QByteArray Boundary = "--------------------ololo----------epepe--------------------";
        r.content_type(("multipart/byteranges; boundary=" + Boundary).constData());
        foreach (const Range &p, list) {
            ba += Boundary + "\r\n";
            ba += "Content-Range: bytes " + QString::number(p.first).toLatin1() + "-"
                    + QString::number(p.second).toLatin1() + "/" + QString::number(data.size()).toLatin1()
                    + "\r\n\r\n";
            ba += data.mid(p.first, (p.second - p.first) + 1) + "\r\n" + Boundary + "\r\n";
        }
    } else if (!list.isEmpty()) {
        r.status(206);
        const Range &p = list.first();
        ba = data.mid(p.first, (p.second - p.first) + 1);
        QString s = "bytes " + QString::number(p.first) + "-" + QString::number(p.second) + "/"
                + QString::number(data.size());
        r.content_range(Tools::toStd(s));
    } else {
        ba = data;
    }
    r.content_length(ba.size());
    r.out().write(ba, ba.size());
    r.out().flush();
}
