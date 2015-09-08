#ifndef CACHE_H
#define CACHE_H

class BTranslator;

class QByteArray;
class QLocale;

#include "global.h"
#include "controller/baseboard.h"
#include "stored/thread.h"
#include "tools.h"

#include <BCoreApplication>
#include <BeQt>

#include <QList>
#include <QMap>
#include <QString>
#include <QStringList>

namespace Cache
{

struct File
{
    QByteArray data;
    qint64 msecsSinceEpoch;
};

typedef void (*ClearCacheFunction)();
typedef void (*SetMaxCacheSizeFunction)(int size);
typedef QMap<QString, ClearCacheFunction> ClearCacheFunctionMap;
typedef QList<Tools::CustomLinkInfo> CustomLinkInfoList;
typedef QMap<QString, SetMaxCacheSizeFunction> SetMaxCacheSizeFunctionMap;
typedef QList<Tools::IpBanInfo> IpBanInfoList;
typedef QList<Post> PostList;

const int defaultCustomContentCacheSize = 10 * BeQt::Megabyte;
const int defaultCustomLinksCacheSize = 100;
const int defaultDynamicFilesCacheSize = 100 * BeQt::Megabyte;
const int defaultFriendListCacheSize = 1 * BeQt::Megabyte;
const int defaultIpBanInfoListCacheSize = 10 * BeQt::Megabyte;
const int defaultLastNPostsCacheSize = 1000;
const int defaultNewsCacheSize = 10 * BeQt::Megabyte;
const int defaultOpPostsCacheSize = 1000;
const int defaultPostsCacheSize = 100 * 1000;
const int defaultRulesCacheSize = 10 * BeQt::Megabyte;
const int defaultStaticFilesCacheSize = 100 * BeQt::Megabyte;
const int defaultThreadPostsCacheSize = 100 * 1000;
const int defaultTranslationsCacheSize = 100;

OLOLORD_EXPORT bool addThreadPost(const QString &boardName, quint64 threadNumber, const Post &post);
OLOLORD_EXPORT bool addLastNPost(const QString &boardName, quint64 threadNumber, const Post &post);
OLOLORD_EXPORT QStringList availableCacheNames();
OLOLORD_EXPORT ClearCacheFunctionMap availableClearCacheFunctions();
OLOLORD_EXPORT SetMaxCacheSizeFunctionMap availableSetMaxCacheSizeFunctions();
OLOLORD_EXPORT bool cacheCustomContent(const QString &prefix, const QLocale &l, QString *content);
OLOLORD_EXPORT bool cacheCustomLinks(const QLocale &l, CustomLinkInfoList *list);
OLOLORD_EXPORT File *cacheDynamicFile(const QString &path, const QByteArray &file);
OLOLORD_EXPORT bool cacheFriendList(Tools::FriendList *list);
OLOLORD_EXPORT bool cacheIpBanInfoList(IpBanInfoList *list);
OLOLORD_EXPORT bool cacheLastNPosts(const QString &boardName, quint64 threadNumber, PostList *list);
OLOLORD_EXPORT bool cacheNews(const QLocale &locale, QStringList *news);
OLOLORD_EXPORT bool cacheOpPost(const QString &boardName, quint64 threadNumber, Post *post);
OLOLORD_EXPORT bool cachePost(const QString &boardName, quint64 postNumber, Content::Post *post);
OLOLORD_EXPORT bool cacheRules(const QString &prefix, const QLocale &locale, QStringList *rules);
OLOLORD_EXPORT File *cacheStaticFile(const QString &path, const QByteArray &file);
OLOLORD_EXPORT bool cacheThreadPosts(const QString &boardName, quint64 threadNumber, PostList *list);
OLOLORD_EXPORT bool cacheTranslator(const QString &name, const QLocale &locale, BTranslator *t);
OLOLORD_EXPORT bool clearCache(const QString &name, QString *err = 0, const QLocale &l = BCoreApplication::locale());
OLOLORD_EXPORT void clearCustomContent();
OLOLORD_EXPORT void clearCustomLinks();
OLOLORD_EXPORT void clearDynamicFilesCache();
OLOLORD_EXPORT void clearFriendListCache();
OLOLORD_EXPORT void clearIpBanInfoListCache();
OLOLORD_EXPORT void clearLastNPostsCache();
OLOLORD_EXPORT void clearNewsCache();
OLOLORD_EXPORT void clearOpPostsCache();
OLOLORD_EXPORT void clearPostsCache();
OLOLORD_EXPORT void clearRulesCache();
OLOLORD_EXPORT void clearStaticFilesCache();
OLOLORD_EXPORT void clearThreadPostsCache();
OLOLORD_EXPORT void clearTranslatorsCache();
OLOLORD_EXPORT QString *customContent(const QString &prefix, const QLocale &l);
OLOLORD_EXPORT CustomLinkInfoList *customLinks(const QLocale &l);
OLOLORD_EXPORT int defaultCacheSize(const QString &name);
OLOLORD_EXPORT File *dynamicFile(const QString &path);
OLOLORD_EXPORT Tools::FriendList *friendList();
OLOLORD_EXPORT IpBanInfoList *ipBanInfoList();
OLOLORD_EXPORT PostList *lastNPosts(const QString &boardName, quint64 threadNumber);
OLOLORD_EXPORT QStringList *news(const QLocale &locale);
OLOLORD_EXPORT Post *opPost(const QString &boardName, quint64 threadNumber);
OLOLORD_EXPORT Content::Post *post(const QString &boardName, quint64 postNumber);
OLOLORD_EXPORT void removeLastNPost(const QString &boardName, quint64 threadNumber, quint64 postNumber);
OLOLORD_EXPORT void removeLastNPosts(const QString &boardName, quint64 threadNumber);
OLOLORD_EXPORT void removeOpPost(const QString &boardName, quint64 threadNumber);
OLOLORD_EXPORT void removePost(const QString &boardName, quint64 postNumber);
OLOLORD_EXPORT void removeThreadPost(const QString &boardName, quint64 threadNumber, quint64 postNumber);
OLOLORD_EXPORT void removeThreadPosts(const QString &boardName, quint64 threadNumber);
OLOLORD_EXPORT QStringList *rules(const QLocale &locale, const QString &prefix);
OLOLORD_EXPORT void setCustomContentMaxCacheSize(int size);
OLOLORD_EXPORT void setCustomLinksMaxCacheSize(int size);
OLOLORD_EXPORT void setDynamicFilesMaxCacheSize(int size);
OLOLORD_EXPORT void setFriendListMaxCacheSize(int size);
OLOLORD_EXPORT void setIpBanInfoListMaxCacheSize(int size);
OLOLORD_EXPORT void setLastNPostsMaxCacheSize(int size);
OLOLORD_EXPORT bool setMaxCacheSize(const QString &name, int size, QString *err = 0,
                                    const QLocale &l = BCoreApplication::locale());
OLOLORD_EXPORT void setNewsMaxCacheSize(int size);
OLOLORD_EXPORT void setOpPostsMaxCacheSize(int size);
OLOLORD_EXPORT void setPostsMaxCacheSize(int size);
OLOLORD_EXPORT void setRulesMaxCacheSize(int size);
OLOLORD_EXPORT void setStaticFilesMaxCacheSize(int size);
OLOLORD_EXPORT void setThreadPostsMaxCacheSize(int size);
OLOLORD_EXPORT void setTranslatorsMaxCacheSize(int size);
OLOLORD_EXPORT File *staticFile(const QString &path);
OLOLORD_EXPORT PostList *threadPosts(const QString &boardName, quint64 threadNumber);
OLOLORD_EXPORT BTranslator *translator(const QString &name, const QLocale &locale);
OLOLORD_EXPORT bool updateLastNPost(const QString &boardName, quint64 threadNumber, const Post &post);
OLOLORD_EXPORT bool updateThreadPost(const QString &boardName, quint64 threadNumber, const Post &post);

}

#endif // CACHE_H
