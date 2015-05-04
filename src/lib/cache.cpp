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
#include <QReadLocker>
#include <QReadWriteLock>
#include <QSettings>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QWriteLocker>

namespace Cache
{

static QCache<QString, QString> customHomePageContents;
static QReadWriteLock customHomePageContentsLock(QReadWriteLock::Recursive);
static QCache<QString, QByteArray> dynamicFiles;
static QReadWriteLock dynamicFilesLock(QReadWriteLock::Recursive);
static QCache<QString, Tools::FriendList> theFriendList;
static QReadWriteLock friendListLock(QReadWriteLock::Recursive);
static QCache<QString, IpBanInfoList> theIpBanInfoList;
static QReadWriteLock ipBanInfoListLock(QReadWriteLock::Recursive);
static QCache<QString, QStringList> theNews;
static QReadWriteLock newsLock(QReadWriteLock::Recursive);
static QCache<QString, Content::Post> thePosts;
static QReadWriteLock postsLock(QReadWriteLock::Recursive);
static QCache<QString, QStringList> theRules;
static QReadWriteLock rulesLock(QReadWriteLock::Recursive);
static QCache<QString, QByteArray> staticFiles;
static QReadWriteLock staticFilesLock(QReadWriteLock::Recursive);
static QCache<QString, BTranslator> translators;
static QReadWriteLock translatorsLock(QReadWriteLock::Recursive);

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
    QWriteLocker locker(&customHomePageContentsLock);
    do_once(init)
        initCache(customHomePageContents, "custom_home_page_content", defaultCustomHomePageContentsCacheSize);
    int sz = content->length() * 2;
    if (customHomePageContents.maxCost() < sz)
        return false;
    customHomePageContents.insert(l.name(), content, sz);
    return true;
}

bool cacheDynamicFile(const QString &path, QByteArray *file)
{
    if (path.isEmpty() || !file)
        return false;
    QWriteLocker locker(&dynamicFilesLock);
    do_once(init)
        initCache(dynamicFiles, "dynamic_files", defaultDynamicFilesCacheSize);
    if (dynamicFiles.maxCost() < file->size())
        return false;
    dynamicFiles.insert(path, file, file->size());
    return true;
}

bool cacheFriendList(Tools::FriendList *list)
{
    if (!list)
        return false;
    QWriteLocker locker(&friendListLock);
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
    QWriteLocker locker(&ipBanInfoListLock);
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
    QWriteLocker locker(&newsLock);
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
    QWriteLocker locker(&postsLock);
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
    QWriteLocker locker(&rulesLock);
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

bool cacheStaticFile(const QString &path, QByteArray *file)
{
    if (path.isEmpty() || !file)
        return false;
    QWriteLocker locker(&staticFilesLock);
    do_once(init)
        initCache(staticFiles, "static_files", defaultStaticFilesCacheSize);
    if (staticFiles.maxCost() < file->size())
        return false;
    staticFiles.insert(path, file, file->size());
    return true;
}

bool cacheTranslator(const QString &name, const QLocale &locale, BTranslator *t)
{
    if (name.isEmpty() || !t)
        return false;
    QWriteLocker locker(&translatorsLock);
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
    QWriteLocker locker(&customHomePageContentsLock);
    customHomePageContents.clear();
}

void clearDynamicFilesCache()
{
    QWriteLocker locker(&dynamicFilesLock);
    dynamicFiles.clear();
}

void clearFriendListCache()
{
    QWriteLocker locker(&friendListLock);
    theFriendList.clear();
}

void clearIpBanInfoListCache()
{
    QWriteLocker locker(&ipBanInfoListLock);
    theIpBanInfoList.clear();
}

void clearNewsCache()
{
    QWriteLocker locker(&newsLock);
    theNews.clear();
}

void clearPostsCache()
{
    QWriteLocker locker(&postsLock);
    thePosts.clear();
}

void clearRulesCache()
{
    QWriteLocker locker(&rulesLock);
    theRules.clear();
}

void clearStaticFilesCache()
{
    QWriteLocker locker(&staticFilesLock);
    staticFiles.clear();
}

void clearTranslatorsCache()
{
    QWriteLocker locker(&translatorsLock);
    translators.clear();
}

QString *customHomePageContent(const QLocale &l)
{
    QReadLocker locker(&customHomePageContentsLock);
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
    QReadLocker locker(&dynamicFilesLock);
    return dynamicFiles.object(path);
}

Tools::FriendList *friendList()
{
    QReadLocker locker(&friendListLock);
    return theFriendList.object("x");
}

IpBanInfoList *ipBanInfoList()
{
    QReadLocker locker(&ipBanInfoListLock);
    return theIpBanInfoList.object("x");
}

QStringList *news(const QLocale &locale)
{
    QReadLocker locker(&newsLock);
    return theNews.object(locale.name());
}

Content::Post *post(const QString &boardName, quint64 postNumber)
{
    if (boardName.isEmpty() || !postNumber)
        return 0;
    QReadLocker locker(&postsLock);
    return thePosts.object(boardName + "/" + QString::number(postNumber));
}

void removePost(const QString &boardName, quint64 postNumber)
{
    if (boardName.isEmpty() || !postNumber)
        return;
    QWriteLocker locker(&postsLock);
    thePosts.remove(boardName + "/" + QString::number(postNumber));
}

QStringList *rules(const QLocale &locale, const QString &prefix)
{
    if (prefix.isEmpty())
        return 0;
    QReadLocker locker(&rulesLock);
    return theRules.object(prefix + "/" + locale.name());
}

void setCustomHomePageContentMaxCacheSize(int size)
{
    if (size < 0)
        return;
    QWriteLocker locker(&customHomePageContentsLock);
    customHomePageContents.setMaxCost(size);
}

void setDynamicFilesMaxCacheSize(int size)
{
    if (size < 0)
        return;
    QWriteLocker locker(&dynamicFilesLock);
    dynamicFiles.setMaxCost(size);
}

void setFriendListMaxCacheSize(int size)
{
    if (size < 0)
        return;
    QWriteLocker locker(&friendListLock);
    theFriendList.setMaxCost(size);
}

void setIpBanInfoListMaxCacheSize(int size)
{
    if (size < 0)
        return;
    QWriteLocker locker(&ipBanInfoListLock);
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
    QWriteLocker locker(&newsLock);
    theNews.setMaxCost(size);
}


void setPostsMaxCacheSize(int size)
{
    if (size < 0)
        return;
    QWriteLocker locker(&postsLock);
    thePosts.setMaxCost(size);
}

void setRulesMaxCacheSize(int size)
{
    if (size < 0)
        return;
    QWriteLocker locker(&rulesLock);
    theRules.setMaxCost(size);
}

void setStaticFilesMaxCacheSize(int size)
{
    if (size < 0)
        return;
    QWriteLocker locker(&staticFilesLock);
    staticFiles.setMaxCost(size);
}

void setTranslatorsMaxCacheSize(int size)
{
    if (size < 0)
        return;
    QWriteLocker locker(&translatorsLock);
    translators.setMaxCost(size);
}

QByteArray *staticFile(const QString &path)
{
    if (path.isEmpty())
        return 0;
    QReadLocker locker(&staticFilesLock);
    return staticFiles.object(path);
}

BTranslator *translator(const QString &name, const QLocale &locale)
{
    if (name.isEmpty())
        return 0;
    QReadLocker locker(&translatorsLock);
    return translators.object(name + "_" + locale.name());
}

}
