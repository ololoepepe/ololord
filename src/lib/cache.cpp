#include "cache.h"

#include "settingslocker.h"
#include "translator.h"

#include <BeQt>
#include <BTranslator>

#include <QByteArray>
#include <QCache>
#include <QLocale>
#include <QMap>
#include <QMutex>
#include <QMutexLocker>
#include <QSettings>
#include <QString>
#include <QStringList>
#include <QVariant>

namespace Cache
{

static QCache<QString, QByteArray> dynamicFiles;
static QMutex dynamicFilesMutex(QMutex::Recursive);
static QCache<QString, QStringList> theRules;
static QMutex rulesMutex(QMutex::Recursive);
static QCache<QString, QByteArray> staticFiles;
static QMutex staticFilesMutex(QMutex::Recursive);
static QCache<QString, BTranslator> translators;
static QMutex translatorsMutex(QMutex::Recursive);

template <typename T>
void initCache(T &cache, const QString &name, int defaultSize)
{
    if (name.isEmpty())
        return;
    if (defaultSize < 0)
        defaultSize = 100 * BeQt::Megabyte;
    int sz = SettingsLocker()->value("Cache/" + name + "/max_size", defaultSize).toInt();
    cache.setMaxCost((sz >= 0) ? sz : 0);
}

ClearCacheFunctionMap availableClearCacheFunctions()
{
    init_once(ClearCacheFunctionMap, map, ClearCacheFunctionMap()) {
        map.insert("dynamic_files", &clearDynamicFilesCache);
        map.insert("rules", &clearRulesCache);
        map.insert("static_files", &clearStaticFilesCache);
        map.insert("translators", &clearTranslatorsCache);
    }
    return map;
}

SetMaxCacheSizeFunctionMap availableSetMaxCacheSizeFunctions()
{
    init_once(SetMaxCacheSizeFunctionMap, map, SetMaxCacheSizeFunctionMap()) {
        map.insert("dynamic_files", &setDynamicFilesMaxCacheSize);
        map.insert("rules", &setRulesMaxCacheSize);
        map.insert("static_files", &setStaticFilesMaxCacheSize);
        map.insert("translators", &setTranslatorsMaxCacheSize);
    }
    return map;
}

QStringList availableCacheNames()
{
    init_once(QStringList, names, QStringList()) {
        names << "dynamic_files";
        names << "rules";
        names << "static_files";
        names << "translators";
    }
    return names;
}

bool cacheDynamicFile(const QString &path, QByteArray *data)
{
    if (path.isEmpty() || !data)
        return false;
    QMutexLocker locker(&dynamicFilesMutex);
    do_once(init)
        initCache(dynamicFiles, "dynamic_files", 100 * BeQt::Megabyte);
    if (dynamicFiles.maxCost() < data->size())
        return false;
    dynamicFiles.insert(path, data, data->size());
    return true;
}

bool cacheRules(const QString &prefix, const QLocale &locale, QStringList *rules)
{
    if (prefix.isEmpty() || !rules)
        return false;
    QMutexLocker locker(&rulesMutex);
    do_once(init)
        initCache(theRules, "rules", 10 * BeQt::Megabyte);
    int sz = 0;
    foreach (const QString &r, *rules)
        sz = r.length() * 2;
    if (theRules.maxCost() < sz)
        return false;
    theRules.insert(prefix + "/" + locale.name(), rules, sz);
    return true;
}

bool cacheStaticFile(const QString &path, QByteArray *data)
{
    if (path.isEmpty() || !data)
        return false;
    QMutexLocker locker(&staticFilesMutex);
    do_once(init)
        initCache(staticFiles, "static_files", 100 * BeQt::Megabyte);
    if (staticFiles.maxCost() < data->size())
        return false;
    staticFiles.insert(path, data, data->size());
    return true;
}

bool cacheTranslator(const QString &name, const QLocale &locale, BTranslator *t)
{
    if (name.isEmpty() || !t)
        return false;
    QMutexLocker locker(&translatorsMutex);
    do_once(init)
        initCache(translators, "translators", 100);
    if (translators.maxCost() < 1)
        return false;
    translators.insert(name + "_" + locale.name(), t, 1);
    return true;
}

bool clearCache(const QString &name, QString *err, const QLocale &l)
{
    ClearCacheFunction f = availableClearCacheFunctions().value(name);
    TranslatorQt tq(l);
    if (!f)
        return bRet(err, tq.translate("clearCache", "No such cache", "error"), false);
    f();
    return bRet(err, QString(), true);
}

void clearDynamicFilesCache()
{
    QMutexLocker locker(&dynamicFilesMutex);
    dynamicFiles.clear();
}

void clearRulesCache()
{
    QMutexLocker locker(&rulesMutex);
    theRules.clear();
}

void clearStaticFilesCache()
{
    QMutexLocker locker(&staticFilesMutex);
    staticFiles.clear();
}

void clearTranslatorsCache()
{
    QMutexLocker locker(&translatorsMutex);
    translators.clear();
}

int defaultCacheSize(const QString &name)
{
    typedef QMap<QString, int> IntMap;
    init_once(IntMap, map, IntMap()) {
        map.insert("dynamic_files", defaultDynamicFilesCacheSize);
        map.insert("rules", defaultRulesCacheSize);
        map.insert("static_files", defaultStaticFilesCacheSize);
        map.insert("translators", defaultTranslationsCacheSize);
    }
    return map.value(name);
}

QByteArray *dynamicFile(const QString &path)
{
    if (path.isEmpty())
        return 0;
    QMutexLocker locker(&dynamicFilesMutex);
    return dynamicFiles.object(path);
}

QStringList *rules(const QLocale &locale, const QString &prefix)
{
    if (prefix.isEmpty())
        return 0;
    QMutexLocker locker(&rulesMutex);
    return theRules.object(prefix + "/" + locale.name());
}

void setDynamicFilesMaxCacheSize(int size)
{
    if (size < 0)
        return;
    QMutexLocker locker(&dynamicFilesMutex);
    dynamicFiles.setMaxCost(size);
}

bool setMaxCacheSize(const QString &name, int size, QString *err, const QLocale &l)
{
    TranslatorQt tq(l);
    if (size < 0)
        return bRet(err, tq.translate("setMaxCacheSize", "Invalid cache size", "error"), false);
    SetMaxCacheSizeFunction f = availableSetMaxCacheSizeFunctions().value(name);
    if (!f)
        return bRet(err, tq.translate("setMaxCacheSize", "No such cache", "error"), false);
    f(size);
    return bRet(err, QString(), true);
}

void setRulesMaxCacheSize(int size)
{
    if (size < 0)
        return;
    QMutexLocker locker(&rulesMutex);
    theRules.setMaxCost(size);
}

void setStaticFilesMaxCacheSize(int size)
{
    if (size < 0)
        return;
    QMutexLocker locker(&staticFilesMutex);
    staticFiles.setMaxCost(size);
}

void setTranslatorsMaxCacheSize(int size)
{
    if (size < 0)
        return;
    QMutexLocker locker(&translatorsMutex);
    translators.setMaxCost(size);
}

QByteArray *staticFile(const QString &path)
{
    if (path.isEmpty())
        return 0;
    QMutexLocker locker(&staticFilesMutex);
    return staticFiles.object(path);
}

BTranslator *translator(const QString &name, const QLocale &locale)
{
    if (name.isEmpty())
        return 0;
    QMutexLocker locker(&translatorsMutex);
    return translators.object(name + "_" + locale.name());
}

}
