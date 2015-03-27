#ifndef ABSTRACTBOARD_H
#define ABSTRACTBOARD_H

namespace Content
{

class Board;
class Post;
class Thread;

}

class Post;

namespace Tools
{

class FileTransaction;

}

class QLocale;
class QString;
class QStringList;

namespace cppcms
{

class application;

namespace http
{

class request;

}

}

#include "../global.h"
#include "tools.h"

#include <QByteArray>
#include <QList>
#include <QMap>
#include <QMutex>
#include <QString>

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
    struct FileInfo
    {
        QString name;
        QByteArray hash;
        QString mimeType;
        int size;
        int height;
        int width;
        QString thumbName;
        int thumbHeight;
        int thumbWidth;
    };
    class OLOLORD_EXPORT FileTransaction
    {
    public:
        AbstractBoard * const Board;
    public:
        bool commited;
        QList<FileInfo> minfos;
    public:
        explicit FileTransaction(AbstractBoard *board);
        ~FileTransaction();
    public:
        void commit();
        QList<FileInfo> fileInfos() const;
        void addInfo(const QString &mainFileName = QString(), const QByteArray &hash = QByteArray(),
                     const QString &mimeType = QString(), int size = 0);
        void setMainFile(const QString &fn, const QByteArray &hash, const QString &mimeType, int size);
        void setMainFileSize(int height, int width);
        void setThumbFile(const QString &fn);
        void setThumbFileSize(int height, int width);
    };
public:
    typedef std::list<BoardInfo> BoardInfoList;
public:
    static const QString defaultFileTypes;
private:
    static QMap<QString, AbstractBoard *> boards;
    static bool boardsInitialized;
    static QMutex boardsMutex;
private:
    QMap<QString, unsigned int> captchaQuotaMap;
    mutable QMutex captchaQuotaMutex;
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
    virtual void beforeStoring(Post *post, const Tools::PostParameters &params, bool thread);
    unsigned int bumpLimit() const;
    unsigned int captchaQuota() const;
    unsigned int captchaQuota(const QString &ip) const;
    void captchaSolved(const QString &ip);
    void captchaUsed(const QString &ip);
    virtual void createPost(cppcms::application &app);
    virtual void createThread(cppcms::application &app);
    virtual QString defaultUserName(const QLocale &l) const;
    bool draftsEnabled() const;
    virtual void handleBoard(cppcms::application &app, unsigned int page = 0);
    virtual void handleRules(cppcms::application &app);
    virtual void handleThread(cppcms::application &app, quint64 threadNumber);
    virtual bool isCaptchaValid(const cppcms::http::request &req, const Tools::PostParameters &params,
                                QString &error) const;
    bool isEnabled() const;
    bool isFileTypeSupported(const QString &mimeType) const;
    bool isFileTypeSupported(const QByteArray &data) const;
    virtual bool isHidden() const;
    virtual QString name() const = 0;
    virtual QStringList postformRules(const QLocale &l) const;
    virtual bool postingEnabled() const;
    unsigned int postLimit() const;
    virtual QStringList rules(const QLocale &l) const;
    virtual bool saveFile(const Tools::File &f, FileTransaction &ft);
    virtual bool showWhois() const;
    virtual QString supportedFileTypes() const;
    virtual bool testParams(const Tools::PostParameters &params, bool post, const QLocale &l,
                            QString *error = 0) const;
    unsigned int threadLimit() const;
    unsigned int threadsPerPage() const;
    virtual QString title(const QLocale &l) const = 0;
    virtual Content::Post toController(const Post &post, const cppcms::http::request &req, bool *ok = 0,
                                       QString *error = 0) const;
protected:
    virtual void beforeRenderBoard(const cppcms::http::request &req, Content::Board *c);
    virtual void beforeRenderThread(const cppcms::http::request &req, Content::Thread *c);
    virtual Content::Board *createBoardController(const cppcms::http::request &req, QString &viewName);
    virtual Content::Thread *createThreadController(const cppcms::http::request &req, QString &viewName);
private:
    static void cleanupBoards();
    static void initBoards(bool reinit = false);
};

#endif // ABSTRACTBOARD_H
