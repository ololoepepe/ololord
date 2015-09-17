#include "cache.h"

#include "controller/baseboard.h"
#include "settingslocker.h"
#include "stored/thread.h"
#include "translator.h"

#include <BeQt>
#include <BTranslator>

#include <QByteArray>
#include <QCache>
#include <QDateTime>
#include <QDebug>
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

static QCache<QString, QString> theCustomContent;
static QReadWriteLock customContentLock(QReadWriteLock::Recursive);
static QCache<QString, CustomLinkInfoList> theCustomLinks;
static QReadWriteLock customLinksLock(QReadWriteLock::Recursive);
static QCache<QString, File> dynamicFiles;
static QReadWriteLock dynamicFilesLock(QReadWriteLock::Recursive);
static QCache<QString, Tools::FriendList> theFriendList;
static QReadWriteLock friendListLock(QReadWriteLock::Recursive);
static QCache<QString, IpBanInfoList> theIpBanInfoList;
static QReadWriteLock ipBanInfoListLock(QReadWriteLock::Recursive);
static QCache<QString, PostList> theLastNPosts;
static QReadWriteLock lastNPostsLock(QReadWriteLock::Recursive);
static QCache<QString, QStringList> theNews;
static QReadWriteLock newsLock(QReadWriteLock::Recursive);
static QCache<QString, Post> theOpPosts;
static QReadWriteLock opPostsLock(QReadWriteLock::Recursive);
static QCache<QString, Content::Post> thePosts;
static QReadWriteLock postsLock(QReadWriteLock::Recursive);
static QCache<QString, QStringList> theRules;
static QReadWriteLock rulesLock(QReadWriteLock::Recursive);
static QCache<QString, File> staticFiles;
static QReadWriteLock staticFilesLock(QReadWriteLock::Recursive);
static QCache<QString, PostList> theThreadPosts;
static QReadWriteLock threadPostsLock(QReadWriteLock::Recursive);
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

bool addThreadPost(const QString &boardName, quint64 threadNumber, const Post &post)
{
    if (boardName.isEmpty() || !threadNumber)
        return false;
    QWriteLocker locker(&threadPostsLock);
    PostList *list = theThreadPosts.object(boardName + "/" + QString::number(threadNumber));
    if (!list)
        return false;
    *list << post;
    return true;
}

bool addLastNPost(const QString &boardName, quint64 threadNumber, const Post &post)
{
    if (boardName.isEmpty() || !threadNumber)
        return false;
    QWriteLocker locker(&lastNPostsLock);
    PostList *list = theLastNPosts.object(boardName + "/" + QString::number(threadNumber));
    if (!list)
        return false;
    list->prepend(post);
    int count = 0;
    foreach (const Post &p, *list) {
        if (!p.draft())
            ++count;
    }
    if (count > 3)
        list->removeLast();
    return true;
}

ClearCacheFunctionMap availableClearCacheFunctions()
{
    init_once(ClearCacheFunctionMap, map, ClearCacheFunctionMap()) {
        map.insert("custom_content", &clearCustomContent);
        map.insert("custom_links", &clearCustomLinks);
        map.insert("dynamic_files", &clearDynamicFilesCache);
        map.insert("friend_list", &clearFriendListCache);
        map.insert("ip_ban_info_list", &clearIpBanInfoListCache);
        map.insert("last_n_posts", &clearLastNPostsCache);
        map.insert("news", &clearNewsCache);
        map.insert("op_posts", &clearOpPostsCache);
        map.insert("posts", &clearPostsCache);
        map.insert("rules", &clearRulesCache);
        map.insert("static_files", &clearStaticFilesCache);
        map.insert("thread_posts", &clearThreadPostsCache);
        map.insert("translators", &clearTranslatorsCache);
    }
    return map;
}

SetMaxCacheSizeFunctionMap availableSetMaxCacheSizeFunctions()
{
    init_once(SetMaxCacheSizeFunctionMap, map, SetMaxCacheSizeFunctionMap()) {
        map.insert("custom_content", &setCustomContentMaxCacheSize);
        map.insert("custom_links", &setCustomLinksMaxCacheSize);
        map.insert("dynamic_files", &setDynamicFilesMaxCacheSize);
        map.insert("friend_list", &setFriendListMaxCacheSize);
        map.insert("ip_ban_info_list", &setIpBanInfoListMaxCacheSize);
        map.insert("last_n_posts", &setLastNPostsMaxCacheSize);
        map.insert("news", &setNewsMaxCacheSize);
        map.insert("op_posts", &setOpPostsMaxCacheSize);
        map.insert("posts", &setPostsMaxCacheSize);
        map.insert("rules", &setRulesMaxCacheSize);
        map.insert("static_files", &setStaticFilesMaxCacheSize);
        map.insert("thread_posts", &setThreadPostsMaxCacheSize);
        map.insert("translators", &setTranslatorsMaxCacheSize);
    }
    return map;
}

QStringList availableCacheNames()
{
    init_once(QStringList, names, QStringList()) {
        names << "custom_content";
        names << "custom_links";
        names << "home_page";
        names << "dynamic_files";
        names << "friend_list";
        names << "ip_ban_info_list";
        names << "last_n_posts";
        names << "news";
        names << "op_posts";
        names << "posts";
        names << "rules";
        names << "static_files";
        names << "thread_posts";
        names << "translators";
    }
    return names;
}

bool cacheCustomContent(const QString &prefix, const QLocale &l, QString *content)
{
    if (prefix.isEmpty() || !content)
        return false;
    QWriteLocker locker(&customContentLock);
    do_once(init)
        initCache(theCustomContent, "custom_content", defaultCustomContentCacheSize);
    int sz = content->length() * 2;
    if (theCustomContent.maxCost() < sz)
        return false;
    theCustomContent.insert(prefix + "/" + l.name(), content, sz);
    return true;
}

bool cacheCustomLinks(const QLocale &l, CustomLinkInfoList *list)
{
    if (!list)
        return false;
    QWriteLocker locker(&customLinksLock);
    do_once(init)
        initCache(theCustomLinks, "custom_links", defaultCustomLinksCacheSize);
    int sz = list->size();
    if (theCustomLinks.maxCost() < sz)
        return false;
    theCustomLinks.insert(l.name(), list, sz);
    return true;
}

File *cacheDynamicFile(const QString &path, const QByteArray &file)
{
    if (path.isEmpty())
        return 0;
    QWriteLocker locker(&dynamicFilesLock);
    do_once(init)
        initCache(dynamicFiles, "dynamic_files", defaultDynamicFilesCacheSize);
    if (dynamicFiles.maxCost() < (file.size() + 8))
        return 0;
    File *f = new File;
    f->data = file;
    f->msecsSinceEpoch = (QDateTime::currentMSecsSinceEpoch() / 1000) * 1000;
    dynamicFiles.insert(path, f, file.size() + 8);
    return f;
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

bool cacheLastNPosts(const QString &boardName, quint64 threadNumber, PostList *list)
{
    if (boardName.isEmpty() || !threadNumber || !list)
        return false;
    QWriteLocker locker(&lastNPostsLock);
    do_once(init)
        initCache(theLastNPosts, "last_n_posts", defaultLastNPostsCacheSize);
    int sz = list->size();
    if (theLastNPosts.maxCost() < sz)
        return false;
    theLastNPosts.insert(boardName + "/" + QString::number(threadNumber), list, sz);
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

bool cacheOpPost(const QString &boardName, quint64 threadNumber, Post *post)
{
    if (boardName.isEmpty() || !threadNumber || !post)
        return false;
    QWriteLocker locker(&opPostsLock);
    do_once(init)
        initCache(theOpPosts, "op_posts", defaultOpPostsCacheSize);
    if (theOpPosts.maxCost() < 1)
        return false;
    theOpPosts.insert(boardName + "/" + QString::number(threadNumber), post, 1);
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

File *cacheStaticFile(const QString &path, const QByteArray &file)
{
    if (path.isEmpty())
        return 0;
    QWriteLocker locker(&staticFilesLock);
    do_once(init)
        initCache(staticFiles, "static_files", defaultStaticFilesCacheSize);
    if (staticFiles.maxCost() < (file.size() + 8))
        return 0;
    File *f = new File;
    f->data = file;
    f->msecsSinceEpoch = (QDateTime::currentMSecsSinceEpoch() / 1000) * 1000;
    staticFiles.insert(path, f, file.size() + 8);
    return f;
}

bool cacheThreadPosts(const QString &boardName, quint64 threadNumber, PostList *list)
{
    if (boardName.isEmpty() || !threadNumber || !list)
        return false;
    QWriteLocker locker(&threadPostsLock);
    do_once(init)
        initCache(theThreadPosts, "thread_posts", defaultThreadPostsCacheSize);
    int sz = list->size();
    if (theThreadPosts.maxCost() < sz)
        return false;
    theThreadPosts.insert(boardName + "/" + QString::number(threadNumber), list, sz);
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

void clearCustomContent()
{
    QWriteLocker locker(&customContentLock);
    theCustomContent.clear();
}

void clearCustomLinks()
{
    QWriteLocker locker(&customLinksLock);
    theCustomLinks.clear();
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

void clearLastNPostsCache()
{
    QWriteLocker locker(&lastNPostsLock);
    theLastNPosts.clear();
}

void clearNewsCache()
{
    QWriteLocker locker(&newsLock);
    theNews.clear();
}

void clearOpPostsCache()
{
    QWriteLocker locker(&opPostsLock);
    theOpPosts.clear();
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

void clearThreadPostsCache()
{
    QWriteLocker locker(&threadPostsLock);
    theThreadPosts.clear();
}

void clearTranslatorsCache()
{
    QWriteLocker locker(&translatorsLock);
    translators.clear();
}

QString *customContent(const QString &prefix, const QLocale &l)
{
    if (prefix.isEmpty())
        return 0;
    QReadLocker locker(&customContentLock);
    return theCustomContent.object(prefix + "/" + l.name());
}

CustomLinkInfoList *customLinks(const QLocale &l)
{
    QReadLocker locker(&customLinksLock);
    return theCustomLinks.object(l.name());
}

int defaultCacheSize(const QString &name)
{
    typedef QMap<QString, int> IntMap;
    init_once(IntMap, map, IntMap()) {
        map.insert("custom_content", defaultCustomContentCacheSize);
        map.insert("custom_links", defaultCustomLinksCacheSize);
        map.insert("dynamic_files", defaultDynamicFilesCacheSize);
        map.insert("friend_list", defaultFriendListCacheSize);
        map.insert("ip_ban_info_list", defaultIpBanInfoListCacheSize);
        map.insert("last_n_posts", defaultLastNPostsCacheSize);
        map.insert("news", defaultNewsCacheSize);
        map.insert("op_posts", defaultOpPostsCacheSize);
        map.insert("rules", defaultRulesCacheSize);
        map.insert("static_files", defaultStaticFilesCacheSize);
        map.insert("thread_posts", defaultThreadPostsCacheSize);
        map.insert("translators", defaultTranslationsCacheSize);
    }
    return map.value(name);
}

File *dynamicFile(const QString &path)
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

PostList *lastNPosts(const QString &boardName, quint64 threadNumber)
{
    if (boardName.isEmpty() || !threadNumber)
        return 0;
    QReadLocker locker(&lastNPostsLock);
    return theLastNPosts.object(boardName + "/" + QString::number(threadNumber));
}

QStringList *news(const QLocale &locale)
{
    QReadLocker locker(&newsLock);
    return theNews.object(locale.name());
}

Post *opPost(const QString &boardName, quint64 threadNumber)
{
    if (boardName.isEmpty() || !threadNumber)
        return 0;
    QReadLocker locker(&opPostsLock);
    return theOpPosts.object(boardName + "/" + QString::number(threadNumber));
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

void removeLastNPost(const QString &boardName, quint64 threadNumber, quint64 postNumber)
{
    if (boardName.isEmpty() || !threadNumber || !postNumber)
        return;
    QWriteLocker locker(&lastNPostsLock);
    PostList *list = theLastNPosts.object(boardName + "/" + QString::number(threadNumber));
    if (!list)
        return;
    foreach (int i, bRangeD(0, list->size() - 1)) {
        if (list->at(i).number() == postNumber) {
            if (list->at(i).draft())
                theLastNPosts.clear();
            else
                list->removeAt(i);
            return;
        }
    }
}

void removeLastNPosts(const QString &boardName, quint64 threadNumber)
{
    if (boardName.isEmpty() || !threadNumber)
        return;
    QWriteLocker locker(&lastNPostsLock);
    theLastNPosts.remove(boardName + "/" + QString::number(threadNumber));
}

void removeOpPost(const QString &boardName, quint64 threadNumber)
{
    if (boardName.isEmpty() || !threadNumber)
        return;
    QWriteLocker locker(&opPostsLock);
    theOpPosts.remove(boardName + "/" + QString::number(threadNumber));
}

void removeThreadPost(const QString &boardName, quint64 threadNumber, quint64 postNumber)
{
    if (boardName.isEmpty() || !threadNumber || !postNumber)
        return;
    QWriteLocker locker(&threadPostsLock);
    PostList *list = theThreadPosts.object(boardName + "/" + QString::number(threadNumber));
    if (!list)
        return;
    foreach (int i, bRangeD(0, list->size() - 1)) {
        if (list->at(i).number() == postNumber) {
            list->removeAt(i);
            return;
        }
    }
}

void removeThreadPosts(const QString &boardName, quint64 threadNumber)
{
    if (boardName.isEmpty() || !threadNumber)
        return;
    QWriteLocker locker(&threadPostsLock);
    theThreadPosts.remove(boardName + "/" + QString::number(threadNumber));
}

QStringList *rules(const QLocale &locale, const QString &prefix)
{
    if (prefix.isEmpty())
        return 0;
    QReadLocker locker(&rulesLock);
    return theRules.object(prefix + "/" + locale.name());
}

void setCustomContentMaxCacheSize(int size)
{
    if (size < 0)
        return;
    QWriteLocker locker(&customContentLock);
    theCustomContent.setMaxCost(size);
}

void setCustomLinksMaxCacheSize(int size)
{
    if (size < 0)
        return;
    QWriteLocker locker(&customLinksLock);
    theCustomLinks.setMaxCost(size);
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

void setLastNPostsMaxCacheSize(int size)
{
    if (size < 0)
        return;
    QWriteLocker locker(&lastNPostsLock);
    theLastNPosts.setMaxCost(size);
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

void setOpPostsMaxCacheSize(int size)
{
    if (size < 0)
        return;
    QWriteLocker locker(&opPostsLock);
    theOpPosts.setMaxCost(size);
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

void setThreadPostsMaxCacheSize(int size)
{
    if (size < 0)
        return;
    QWriteLocker locker(&threadPostsLock);
    theThreadPosts.setMaxCost(size);
}

void setTranslatorsMaxCacheSize(int size)
{
    if (size < 0)
        return;
    QWriteLocker locker(&translatorsLock);
    translators.setMaxCost(size);
}

File *staticFile(const QString &path)
{
    if (path.isEmpty())
        return 0;
    QReadLocker locker(&staticFilesLock);
    return staticFiles.object(path);
}

PostList *threadPosts(const QString &boardName, quint64 threadNumber)
{
    if (boardName.isEmpty() || !threadNumber)
        return 0;
    QReadLocker locker(&threadPostsLock);
    return theThreadPosts.object(boardName + "/" + QString::number(threadNumber));
}

BTranslator *translator(const QString &name, const QLocale &locale)
{
    if (name.isEmpty())
        return 0;
    QReadLocker locker(&translatorsLock);
    return translators.object(name + "_" + locale.name());
}

bool updateLastNPost(const QString &boardName, quint64 threadNumber, const Post &post)
{
    if (boardName.isEmpty() || !threadNumber)
        return false;
    QWriteLocker locker(&lastNPostsLock);
    PostList *list = theLastNPosts.object(boardName + "/" + QString::number(threadNumber));
    if (!list)
        return false;
    foreach (int i, bRangeD(0, list->size() - 1)) {
        if (list->at(i).number() == post.number()) {
            if (list->at(i).draft() != post.draft())
                theLastNPosts.clear();
            else
                list->replace(i, post);
            return true;
        }
    }
    return false;
}

bool updateThreadPost(const QString &boardName, quint64 threadNumber, const Post &post)
{
    if (boardName.isEmpty() || !threadNumber)
        return false;
    QWriteLocker locker(&threadPostsLock);
    PostList *list = theThreadPosts.object(boardName + "/" + QString::number(threadNumber));
    if (!list)
        return false;
    foreach (int i, bRangeD(0, list->size() - 1)) {
        if (list->at(i).number() == post.number()) {
            list->replace(i, post);
            return true;
        }
    }
    return false;
}

}
