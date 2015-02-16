#ifndef ABSTRACTBOARD_H
#define ABSTRACTBOARD_H

class QLocale;
class QString;
class QStringList;

namespace cppcms
{

class application;

}

#include "../global.h"

#include <QMap>
#include <QMutex>

#include <list>
#include <string>

class OLOLORD_EXPORT AbstractBoard
{
public:
    struct BoardInfo
    {
        std::string name;
        std::string title;
    };
public:
    typedef std::list<BoardInfo> BoardInfoList;
private:
    static QMap<QString, AbstractBoard *> boards;
    static bool boardsInitialized;
    static QMutex boardsMutex;
public:
    explicit AbstractBoard();
    virtual ~AbstractBoard();
public:
    static AbstractBoard *board(const QString &name);
    static BoardInfoList boardInfos(const QLocale &l, bool includeHidden = true);
    static QStringList boardNames(bool includeHidden = true);
    static void reloadBoards();
public:
    unsigned int archiveLimit() const;
    QString bannerFileName() const;
    unsigned int bumpLimit() const;
    virtual void createPost(cppcms::application &app);
    virtual void createThread(cppcms::application &app);
    virtual void handleBoard(cppcms::application &app, unsigned int page = 0);
    virtual void handleRules(cppcms::application &app);
    virtual void handleThread(cppcms::application &app, quint64 threadNumber);
    bool isEnabled() const;
    virtual bool isHidden() const;
    virtual QString name() const = 0;
    virtual bool postingEnabled() const;
    unsigned int postLimit() const;
    virtual bool processCode() const;
    virtual QStringList rules(const QLocale &l) const;
    unsigned int threadLimit() const;
    unsigned int threadsPerPage() const;
    virtual QString title(const QLocale &l) const = 0;
private:
    static void cleanupBoards();
    static void initBoards(bool reinit = false);
};

#endif // ABSTRACTBOARD_H
