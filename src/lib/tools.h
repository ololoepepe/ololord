#ifndef TOOLS_H
#define TOOLS_H

class QLocale;

namespace cppcms
{

class application;
class base_content;

namespace http
{

class request;

}

}

namespace std
{

class locale;

}

#include "global.h"

#include <BCoreApplication>

#include <QByteArray>
#include <QChar>
#include <QDateTime>
#include <QImage>
#include <QList>
#include <QMap>
#include <QString>
#include <QStringList>
#include <QVariant>

#include <cppcms/json.h>

#include <list>
#include <string>

namespace Tools
{

struct OLOLORD_EXPORT AudioTags
{
    QString album;
    QString artist;
    QImage cover;
    QString title;
    QString year;
};

struct OLOLORD_EXPORT CustomLinkInfo
{
    QString imgUrl;
    QString target;
    QString text;
    QString url;
};

struct OLOLORD_EXPORT File
{
    QByteArray data;
    QString fileName;
    QString formFieldName;
    QString mimeType;
    int rating;
};

struct OLOLORD_EXPORT Friend
{
    QString name;
    QString title;
    QString url;
};

struct OLOLORD_EXPORT IpRange
{
    unsigned int start;
    unsigned int end;
public:
    explicit IpRange(const QString &text, const QChar &separator = '-');
    explicit IpRange(const QStringList &sl, int startIndex = 0, int endIndex = 1, bool num = false);
public:
    void clear();
    bool in(unsigned int ip) const;
    bool in(const QString &ip) const;
    bool isValid() const;
public:
    bool operator <(const IpRange &other) const;
};

struct OLOLORD_EXPORT IpBanInfo
{
    IpRange range;
    int level;
public:
    explicit IpBanInfo(const QStringList &sl);
public:
    bool isValid() const;
};

struct OLOLORD_EXPORT IsMobile
{
    struct {
        bool device;
        bool phone;
        bool tablet;
    } amazon;
    struct {
        bool device;
        bool phone;
        bool tablet;
    } android;
    struct {
        bool device;
        bool ipod;
        bool phone;
        bool tablet;
    } apple;
    struct {
        bool blackberry;
        bool blackberry10;
        bool device;
        bool firefox;
        bool opera;
    } other;
    struct {
        bool device;
        bool phone;
        bool tablet;
    } windows;
    bool any;
    bool phone;
    bool sevenInch;
    bool tablet;
};

typedef QList<File> FileList;

struct OLOLORD_EXPORT Post
{
    bool draft;
    QString email;
    QStringList fileHashes;
    FileList files;
    QString name;
    QByteArray password;
    bool raw;
    bool showTripcode;
    QString subject;
    QString text;
};

enum MaxInfo
{
    MaxEmailFieldLength = 1,
    MaxNameFieldLength,
    MaxSubjectFieldLength,
    MaxTextFieldLength,
    MaxPasswordFieldLength,
    MaxFileCount,
    MaxFileSize,
    MaxLastPosts
};

typedef QList<Friend> FriendList;
typedef QMap<QString, QString> GetParameters;
typedef QMap<QString, QString> PostParameters;

const QString InputDateTimeFormat = "dd.MM.yyyy:hh";

OLOLORD_EXPORT QStringList acceptedExternalBoards();
OLOLORD_EXPORT AudioTags audioTags(const QString &fileName);
OLOLORD_EXPORT QString captchaQuotaFile();
OLOLORD_EXPORT bool captchaEnabled(const QString &boardName);
OLOLORD_EXPORT QString cookieValue(const cppcms::http::request &req, const QString &name);
OLOLORD_EXPORT QString customContent(const QString &prefix, const QLocale &l);
OLOLORD_EXPORT QList<CustomLinkInfo> customLinks(const QLocale &l);
OLOLORD_EXPORT QDateTime dateTime(const QDateTime &dt, const cppcms::http::request &req);
OLOLORD_EXPORT QString externalLinkRegexpPattern();
OLOLORD_EXPORT bool externalLinkRootZoneExists(const QString &zoneName);
OLOLORD_EXPORT QString flagName(const QString &countryCode);
OLOLORD_EXPORT QVariant fromJson(const cppcms::json::value &v);
OLOLORD_EXPORT QLocale fromStd(const std::locale &l);
OLOLORD_EXPORT QString fromStd(const std::string &s);
OLOLORD_EXPORT QStringList fromStd(const std::list<std::string> &sl);
OLOLORD_EXPORT GetParameters getParameters(const cppcms::http::request &request);
OLOLORD_EXPORT QByteArray hashpass(const cppcms::http::request &req);
OLOLORD_EXPORT QString hashpassString(const cppcms::http::request &req);
OLOLORD_EXPORT int ipBanLevel(const QString &ip);
OLOLORD_EXPORT int ipBanLevel(const cppcms::http::request &req);
OLOLORD_EXPORT bool isAudioType(const QString &mimeType);
OLOLORD_EXPORT bool isImageType(const QString &mimeType);
OLOLORD_EXPORT IsMobile isMobile(const cppcms::http::request &req);
OLOLORD_EXPORT unsigned int ipNum(const QString &ip, bool *ok = 0);
OLOLORD_EXPORT bool isSpecialThumbName(const QString &tn);
OLOLORD_EXPORT bool isVideoType(const QString &mimeType);
OLOLORD_EXPORT QString langName(const QString &id);
OLOLORD_EXPORT QDateTime localDateTime(const QDateTime &dt, int offsetMinutes = -1000);
OLOLORD_EXPORT QLocale locale(const cppcms::http::request &req,
                              const QLocale &defaultLocale = BCoreApplication::locale());
OLOLORD_EXPORT void log(const cppcms::application &app, const QString &action, const QString &state,
                        const QString &target =  QString());
OLOLORD_EXPORT void log(const cppcms::http::request &req, const QString &action, const QString &state,
                        const QString &target =  QString());
OLOLORD_EXPORT void log(const char *where, const std::exception &e);
OLOLORD_EXPORT unsigned int maxInfo(MaxInfo m, const QString &boardName = QString());
OLOLORD_EXPORT QString mimeType(const QByteArray &data, bool *ok = 0);
OLOLORD_EXPORT QStringList news(const QLocale &l);
OLOLORD_EXPORT FileList postFiles(const cppcms::http::request &request, const PostParameters &params,
                                  const QString &baordName, bool *ok = 0, QString *error = 0,
                                  const QLocale &l = BCoreApplication::locale());
OLOLORD_EXPORT PostParameters postParameters(const cppcms::http::request &request);
OLOLORD_EXPORT cppcms::json::value readJsonValue(const QString &fileName, bool *ok = 0);
OLOLORD_EXPORT void render(cppcms::application &app, const QString &templateName, cppcms::base_content &content);
OLOLORD_EXPORT QString renderTo(cppcms::application &app, const QString &templateName, cppcms::base_content &content);
OLOLORD_EXPORT void redirect(cppcms::application &app, const QString &path = QString());
OLOLORD_EXPORT void resetLoggingSkipIps();
OLOLORD_EXPORT QStringList rules(const QString &prefix, const QLocale &l);
OLOLORD_EXPORT QString searchIndexFile();
OLOLORD_EXPORT FriendList siteFriends();
OLOLORD_EXPORT QString storagePath();
OLOLORD_EXPORT QStringList supportedCodeLanguages();
OLOLORD_EXPORT int timeZoneMinutesOffset(const cppcms::http::request &req, int defaultOffset = -1000);
OLOLORD_EXPORT QByteArray toHashpass(const QString &s, bool *ok = 0);
OLOLORD_EXPORT cppcms::json::value toJson(const QVariant &v);
OLOLORD_EXPORT Post toPost(const PostParameters &params, const FileList &files);
OLOLORD_EXPORT Post toPost(const cppcms::http::request &req, const QString &boardName);
OLOLORD_EXPORT std::locale toStd(const QLocale &l);
OLOLORD_EXPORT std::string toStd(const QString &s);
OLOLORD_EXPORT std::list<std::string> toStd(const QStringList &sl);
OLOLORD_EXPORT QString toString(const QByteArray &hp, bool *ok = 0);
OLOLORD_EXPORT QString userIp(const cppcms::http::request &req, bool *proxy = 0);

}

#endif // TOOLS_H
