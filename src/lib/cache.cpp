#include "cache.h"

#include "controller/baseboard.h"
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
static QCache<QString, Content::BaseBoard::File> fileInfos;
static QMutex fileInfosMutex(QMutex::Recursive);
static QCache<QString, IpBanInfoList> theIpBanInfoList;
static QMutex ipBanInfoListMutex(QMutex::Recursive);
static QCache<QString, QStringList> theNews;
static QMutex newsMutex(QMutex::Recursive);
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
        map.insert("file_infos", &clearFileInfosCache);
        map.insert("ip_ban_info_list", &clearIpBanInfoListCache);
        map.insert("news", &clearNewsCache);
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
        map.insert("file_infos", &setFileInfosMaxCacheSize);
        map.insert("ip_ban_info_list", &setIpBanInfoListMaxCacheSize);
        map.insert("news", &setNewsMaxCacheSize);
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
        names << "file_infos";
        names << "ip_ban_info_list";
        names << "news";
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
        initCache(dynamicFiles, "dynamic_files", defaultDynamicFilesCacheSize);
    if (dynamicFiles.maxCost() < data->size())
        return false;
    dynamicFiles.insert(path, data, data->size());
    return true;
}

bool cacheFileInfo(const QString &boardName, const QString &fileName, Content::BaseBoard::File *fi)
{
    if (boardName.isEmpty() || fileName.isEmpty() || !fi)
        return false;
    QMutexLocker locker(&fileInfosMutex);
    do_once(init)
        initCache(fileInfos, "file_infos", defaultFileInfosCacheSize);
    int sz = fi->size.size() + fi->sourceName.size() + fi->thumbName.size() + 2 * sizeof(int);
    if (fileInfos.maxCost() < sz)
        return false;
    fileInfos.insert(boardName + "/" + fileName, fi, sz);
    return true;
}

bool cacheIpBanInfoList(IpBanInfoList *list)
{
    if (!list)
        return false;
    QMutexLocker locker(&ipBanInfoListMutex);
    do_once(init)
        initCache(theIpBanInfoList, "ip_ban_info_list", defaultIpBanInfoListCacheSize);
    int sz = list->size() * 2 * sizeof(int);
    if (theIpBanInfoList.maxCost() < sz)
        return false;
    theIpBanInfoList.insert("x", list, sz);
    return true;
}

bool cacheNews(const QLocale &locale, QStringList *news)
{
    if (!news)
        return false;
    QMutexLocker locker(&newsMutex);
    do_once(init)
        initCache(theNews, "news", defaultNewsCacheSize);
    int sz = 0;
    foreach (const QString &r, *news)
        sz = r.length() * 2;
    if (theNews.maxCost() < sz)
        return false;
    theNews.insert(locale.name(), news, sz);
    return true;
}

bool cacheRules(const QString &prefix, const QLocale &locale, QStringList *rules)
{
    if (prefix.isEmpty() || !rules)
        return false;
    QMutexLocker locker(&rulesMutex);
    do_once(init)
        initCache(theRules, "rules", defaultRulesCacheSize);
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
        initCache(staticFiles, "static_files", defaultStaticFilesCacheSize);
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
        initCache(translators, "translators", defaultTranslationsCacheSize);
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

void clearFileInfosCache()
{
    QMutexLocker locker(&fileInfosMutex);
    fileInfos.clear();
}

void clearIpBanInfoListCache()
{
    QMutexLocker locker(&ipBanInfoListMutex);
    theIpBanInfoList.clear();
}

void clearNewsCache()
{
    QMutexLocker locker(&newsMutex);
    theNews.clear();
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
        map.insert("file_infos", defaultFileInfosCacheSize);
        map.insert("ip_ban_info_list", defaultIpBanInfoListCacheSize);
        map.insert("news", defaultNewsCacheSize);
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

Content::BaseBoard::File *fileInfo(const QString &boardName, const QString &fileName)
{
    if (boardName.isEmpty() || fileName.isEmpty())
        return 0;
    QMutexLocker locker(&fileInfosMutex);
    return fileInfos.object(boardName + "/" + fileName);
}

IpBanInfoList *ipBanInfoList()
{
    QMutexLocker locker(&ipBanInfoListMutex);
    return theIpBanInfoList.object("x");
}

QStringList *news(const QLocale &locale)
{
    QMutexLocker locker(&newsMutex);
    return theNews.object(locale.name());
}

void removeFileInfos(const QString &boardName, const QStringList &fileNames)
{
    if (boardName.isEmpty() || fileNames.isEmpty())
        return;
    QMutexLocker locker(&fileInfosMutex);
    foreach (const QString &fn, fileNames) {
        if (fn.isEmpty())
            continue;
        fileInfos.remove(boardName + "/" + fn);
    }
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

void setFileInfosMaxCacheSize(int size)
{
    if (size < 0)
        return;
    QMutexLocker locker(&fileInfosMutex);
    fileInfos.setMaxCost(size);
}

void setIpBanInfoListMaxCacheSize(int size)
{
    if (size < 0)
        return;
    QMutexLocker locker(&ipBanInfoListMutex);
    theIpBanInfoList.setMaxCost(size);
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

void setNewsMaxCacheSize(int size)
{
    if (size < 0)
        return;
    QMutexLocker locker(&newsMutex);
    theNews.setMaxCost(size);
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
