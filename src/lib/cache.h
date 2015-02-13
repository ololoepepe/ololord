#ifndef CACHE_H
#define CACHE_H

class BTranslator;

class QByteArray;
class QLocale;

#include "global.h"

#include <BCoreApplication>
#include <BeQt>

#include <QMap>
#include <QString>
#include <QStringList>

namespace Cache
{

typedef void (*ClearCacheFunction)();
typedef void (*SetMaxCacheSizeFunction)(int size);
typedef QMap<QString, ClearCacheFunction> ClearCacheFunctionMap;
typedef QMap<QString, SetMaxCacheSizeFunction> SetMaxCacheSizeFunctionMap;

const int defaultDynamicFilesCacheSize = 100 * BeQt::Megabyte;
const int defaultRulesCacheSize = 10 * BeQt::Megabyte;
const int defaultStaticFilesCacheSize = 100 * BeQt::Megabyte;
const int defaultTranslationsCacheSize = 100;

OLOLORD_EXPORT QStringList availableCacheNames();
OLOLORD_EXPORT ClearCacheFunctionMap availableClearCacheFunctions();
OLOLORD_EXPORT SetMaxCacheSizeFunctionMap availableSetMaxCacheSizeFunctions();
OLOLORD_EXPORT bool cacheDynamicFile(const QString &path, QByteArray *data);
OLOLORD_EXPORT bool cacheRules(const QString &prefix, const QLocale &locale, QStringList *rules);
OLOLORD_EXPORT bool cacheStaticFile(const QString &path, QByteArray *data);
OLOLORD_EXPORT bool cacheTranslator(const QString &name, const QLocale &locale, BTranslator *t);
OLOLORD_EXPORT bool clearCache(const QString &name, QString *err = 0, const QLocale &l = BCoreApplication::locale());
OLOLORD_EXPORT void clearDynamicFilesCache();
OLOLORD_EXPORT void clearRulesCache();
OLOLORD_EXPORT void clearStaticFilesCache();
OLOLORD_EXPORT void clearTranslatorsCache();
OLOLORD_EXPORT int defaultCacheSize(const QString &name);
OLOLORD_EXPORT QByteArray *dynamicFile(const QString &path);
OLOLORD_EXPORT QStringList *rules(const QLocale &locale, const QString &prefix);
OLOLORD_EXPORT void setDynamicFilesMaxCacheSize(int size);
OLOLORD_EXPORT bool setMaxCacheSize(const QString &name, int size, QString *err = 0,
                                    const QLocale &l = BCoreApplication::locale());
OLOLORD_EXPORT void setRulesMaxCacheSize(int size);
OLOLORD_EXPORT void setStaticFilesMaxCacheSize(int size);
OLOLORD_EXPORT void setTranslatorsMaxCacheSize(int size);
OLOLORD_EXPORT QByteArray *staticFile(const QString &path);
OLOLORD_EXPORT BTranslator *translator(const QString &name, const QLocale &locale);

}

#endif // CACHE_H
