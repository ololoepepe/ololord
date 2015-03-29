#ifndef CACHE_H
#define CACHE_H

class BTranslator;

class QByteArray;
class QLocale;

#include "global.h"
#include "controller/baseboard.h"
#include "tools.h"

#include <BCoreApplication>
#include <BeQt>

#include <QList>
#include <QMap>
#include <QString>
#include <QStringList>

namespace Cache
{

typedef void (*ClearCacheFunction)();
typedef void (*SetMaxCacheSizeFunction)(int size);
typedef QMap<QString, ClearCacheFunction> ClearCacheFunctionMap;
typedef QMap<QString, SetMaxCacheSizeFunction> SetMaxCacheSizeFunctionMap;
typedef QList<Tools::IpBanInfo> IpBanInfoList;

const int defaultDynamicFilesCacheSize = 100 * BeQt::Megabyte;
const int defaultIpBanInfoListCacheSize = 10 * BeQt::Megabyte;
const int defaultNewsCacheSize = 10 * BeQt::Megabyte;
const int defaultPostsCacheSize = 10 * 1000;
const int defaultRulesCacheSize = 10 * BeQt::Megabyte;
const int defaultStaticFilesCacheSize = 100 * BeQt::Megabyte;
const int defaultTranslationsCacheSize = 100;

OLOLORD_EXPORT QStringList availableCacheNames();
OLOLORD_EXPORT ClearCacheFunctionMap availableClearCacheFunctions();
OLOLORD_EXPORT SetMaxCacheSizeFunctionMap availableSetMaxCacheSizeFunctions();
OLOLORD_EXPORT bool cacheDynamicFile(const QString &path, QByteArray *data);
OLOLORD_EXPORT bool cacheIpBanInfoList(IpBanInfoList *list);
OLOLORD_EXPORT bool cacheNews(const QLocale &locale, QStringList *news);
OLOLORD_EXPORT bool cachePost(const QString &boardName, quint64 postNumber, Content::Post *post);
OLOLORD_EXPORT bool cacheRules(const QString &prefix, const QLocale &locale, QStringList *rules);
OLOLORD_EXPORT bool cacheStaticFile(const QString &path, QByteArray *data);
OLOLORD_EXPORT bool cacheTranslator(const QString &name, const QLocale &locale, BTranslator *t);
OLOLORD_EXPORT bool clearCache(const QString &name, QString *err = 0, const QLocale &l = BCoreApplication::locale());
OLOLORD_EXPORT void clearDynamicFilesCache();
OLOLORD_EXPORT void clearIpBanInfoListCache();
OLOLORD_EXPORT void clearNewsCache();
OLOLORD_EXPORT void clearPostsCache();
OLOLORD_EXPORT void clearRulesCache();
OLOLORD_EXPORT void clearStaticFilesCache();
OLOLORD_EXPORT void clearTranslatorsCache();
OLOLORD_EXPORT int defaultCacheSize(const QString &name);
OLOLORD_EXPORT QByteArray *dynamicFile(const QString &path);
OLOLORD_EXPORT IpBanInfoList *ipBanInfoList();
OLOLORD_EXPORT Content::Post *post(const QString &boardName, quint64 postNumber);
OLOLORD_EXPORT QStringList *news(const QLocale &locale);
OLOLORD_EXPORT void removePost(const QString &boardName, quint64 postNumber);
OLOLORD_EXPORT QStringList *rules(const QLocale &locale, const QString &prefix);
OLOLORD_EXPORT void setDynamicFilesMaxCacheSize(int size);
OLOLORD_EXPORT void setIpBanInfoListMaxCacheSize(int size);
OLOLORD_EXPORT bool setMaxCacheSize(const QString &name, int size, QString *err = 0,
                                    const QLocale &l = BCoreApplication::locale());
OLOLORD_EXPORT void setNewsMaxCacheSize(int size);
OLOLORD_EXPORT void setPostsMaxCacheSize(int size);
OLOLORD_EXPORT void setRulesMaxCacheSize(int size);
OLOLORD_EXPORT void setStaticFilesMaxCacheSize(int size);
OLOLORD_EXPORT void setTranslatorsMaxCacheSize(int size);
OLOLORD_EXPORT QByteArray *staticFile(const QString &path);
OLOLORD_EXPORT BTranslator *translator(const QString &name, const QLocale &locale);

}

#endif // CACHE_H
