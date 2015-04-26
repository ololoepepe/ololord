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

static QCache<QString, QString> customHomePageContents;
static QMutex customHomePageContentsMutex(QMutex::Recursive);
static QCache<QString, QByteArray> dynamicFiles;
static QMutex dynamicFilesMutex(QMutex::Recursive);
static QCache<QString, Tools::FriendList> theFriendList;
static QMutex friendListMutex(QMutex::Recursive);
static QCache<QString, IpBanInfoList> theIpBanInfoList;
static QMutex ipBanInfoListMutex(QMutex::Recursive);
static QCache<QString, QStringList> theNews;
static QMutex newsMutex(QMutex::Recursive);
static QCache<QString, Content::Post> thePosts;
static QMutex postsMutex(QMutex::Recursive);
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
        map.insert("custom_home_page_content", &clearCustomHomePageContent);
        map.insert("dynamic_files", &clearDynamicFilesCache);
        map.insert("friend_list", &clearFriendListCache);
        map.insert("ip_ban_info_list", &clearIpBanInfoListCache);
        map.insert("news", &clearNewsCache);
        map.insert("posts", &clearPostsCache);
        map.insert("rules", &clearRulesCache);
        map.insert("static_files", &clearStaticFilesCache);
        map.insert("translators", &clearTranslatorsCache);
    }
    return map;
}

SetMaxCacheSizeFunctionMap availableSetMaxCacheSizeFunctions()
{
    init_once(SetMaxCacheSizeFunctionMap, map, SetMaxCacheSizeFunctionMap()) {
        map.insert("custom_home_page_content", &setCustomHomePageContentMaxCacheSize);
        map.insert("dynamic_files", &setDynamicFilesMaxCacheSize);
        map.insert("friend_list", &setFriendListMaxCacheSize);
        map.insert("ip_ban_info_list", &setIpBanInfoListMaxCacheSize);
        map.insert("news", &setNewsMaxCacheSize);
        map.insert("posts", &setPostsMaxCacheSize);
        map.insert("rules", &setRulesMaxCacheSize);
        map.insert("static_files", &setStaticFilesMaxCacheSize);
        map.insert("translators", &setTranslatorsMaxCacheSize);
    }
    return map;
}

QStringList availableCacheNames()
{
    init_once(QStringList, names, QStringList()) {
        names << "custom_home_page_content";
        names << "dynamic_files";
        names << "friend_list";
        names << "ip_ban_info_list";
        names << "news";
        names << "posts";
        names << "rules";
        names << "static_files";
        names << "translators";
    }
    return names;
}

bool cacheCustomHomePageContent(const QLocale &l, QString *content)
{
    if (!content)
        return false;
    QMutexLocker locker(&customHomePageContentsMutex);
    do_once(init)
        initCache(customHomePageContents, "custom_home_page_content", defaultCustomHomePageContentsCacheSize);
    int sz = content->length() * 2;
    if (customHomePageContents.maxCost() < sz)
        return false;
    customHomePageContents.insert(l.name(), content, sz);
    return true;
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

bool cacheFriendList(Tools::FriendList *list)
{
    if (!list)
        return false;
    QMutexLocker locker(&friendListMutex);
    do_once(init)
        initCache(theFriendList, "friend_list", defaultFriendListCacheSize);
    int sz = 0;
    foreach (const Tools::Friend &f, *list)
        sz += f.name.length() * 2 + f.title.length() * 2 + f.url.length() * 2;
    if (theFriendList.maxCost() < sz)
        return false;
    theFriendList.insert("x", list, sz);
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

bool cachePost(const QString &boardName, quint64 postNumber, Content::Post *post)
{
    if (boardName.isEmpty() || !postNumber || !post)
        return false;
    QMutexLocker locker(&postsMutex);
    do_once(init)
        initCache(thePosts, "posts", defaultPostsCacheSize);
    if (thePosts.maxCost() < 1)
        return false;
    thePosts.insert(boardName + "/" + QString::number(postNumber), post, 1);
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

void clearCustomHomePageContent()
{
    QMutexLocker locker(&customHomePageContentsMutex);
    customHomePageContents.clear();
}

void clearDynamicFilesCache()
{
    QMutexLocker locker(&dynamicFilesMutex);
    dynamicFiles.clear();
}

void clearFriendListCache()
{
    QMutexLocker locker(&friendListMutex);
    theFriendList.clear();
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

void clearPostsCache()
{
    QMutexLocker locker(&postsMutex);
    thePosts.clear();
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

QString *customHomePageContent(const QLocale &l)
{
    QMutexLocker locker(&customHomePageContentsMutex);
    return customHomePageContents.object(l.name());
}

int defaultCacheSize(const QString &name)
{
    typedef QMap<QString, int> IntMap;
    init_once(IntMap, map, IntMap()) {
        map.insert("custom_home_page_content", defaultCustomHomePageContentsCacheSize);
        map.insert("dynamic_files", defaultDynamicFilesCacheSize);
        map.insert("friend_list", defaultFriendListCacheSize);
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

Tools::FriendList *friendList()
{
    QMutexLocker locker(&friendListMutex);
    return theFriendList.object("x");
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

Content::Post *post(const QString &boardName, quint64 postNumber)
{
    if (boardName.isEmpty() || !postNumber)
        return 0;
    QMutexLocker locker(&postsMutex);
    return thePosts.object(boardName + "/" + QString::number(postNumber));
}

void removePost(const QString &boardName, quint64 postNumber)
{
    if (boardName.isEmpty() || !postNumber)
        return;
    QMutexLocker locker(&postsMutex);
    thePosts.remove(boardName + "/" + QString::number(postNumber));
}

QStringList *rules(const QLocale &locale, const QString &prefix)
{
    if (prefix.isEmpty())
        return 0;
    QMutexLocker locker(&rulesMutex);
    return theRules.object(prefix + "/" + locale.name());
}

void setCustomHomePageContentMaxCacheSize(int size)
{
    if (size < 0)
        return;
    QMutexLocker locker(&customHomePageContentsMutex);
    customHomePageContents.setMaxCost(size);
}

void setDynamicFilesMaxCacheSize(int size)
{
    if (size < 0)
        return;
    QMutexLocker locker(&dynamicFilesMutex);
    dynamicFiles.setMaxCost(size);
}

void setFriendListMaxCacheSize(int size)
{
    if (size < 0)
        return;
    QMutexLocker locker(&friendListMutex);
    theFriendList.setMaxCost(size);
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


void setPostsMaxCacheSize(int size)
{
    if (size < 0)
        return;
    QMutexLocker locker(&postsMutex);
    thePosts.setMaxCost(size);
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
