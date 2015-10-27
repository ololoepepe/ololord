#ifndef ABSTRACTBOARD_H
#define ABSTRACTBOARD_H

namespace Content
{

class Board;
class Catalog;
class EditPost;
class Post;
class Thread;

}

class Post;
class Thread;

namespace Tools
{

class FileTransaction;

}

class QImage;
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
#include <QElapsedTimer>
#include <QList>
#include <QMap>
#include <QMutex>
#include <QReadLocker>
#include <QReadWriteLock>
#include <QSharedPointer>
#include <QString>

#include <cppcms/json.h>

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
        QVariant metaData;
        int rating;
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
                     const QString &mimeType = QString(), int size = 0, int rating = 0);
        void setMainFile(const QString &fn, const QByteArray &hash, const QString &mimeType, int size);
        void setMainFileSize(int height, int width);
        void setThumbFile(const QString &fn);
        void setThumbFileSize(int height, int width);
        void setMetaData(const QVariant &metaData);
    };
    class OLOLORD_EXPORT LockingWrapper
    {
    private:
        AbstractBoard * const Board;
    private:
        QSharedPointer<QReadLocker> locker;
    public:
        LockingWrapper(const LockingWrapper &other);
    private:
        explicit LockingWrapper(AbstractBoard *board);
    public:
        AbstractBoard *data() const;
        bool isNull() const;
    public:
        LockingWrapper &operator =(const LockingWrapper &other);
        AbstractBoard *operator ->() const;
        bool operator !() const;
        operator bool() const;
    private:
        friend class AbstractBoard;
    };
    enum MarkupElement
    {
        NoMarkupElement = 0x000,
        BoldMarkupElement = 0x001,
        ItalicsMarkupElement = 0x002,
        StrikedOutMarkupElement = 0x004,
        UnderlinedMarkupElement = 0x008,
        SpoilerMarkupElement = 0x010,
        QuotationMarkupElement = 0x020,
        CodeMarkupElement = 0x040,
        SubscriptMarkupElement = 0x080,
        SuperscriptMarkupElement = 0x100,
        UrlMarkupElement = 0x200
    };
    Q_DECLARE_FLAGS(MarkupElements, MarkupElement)
    struct PostingSpeed
    {
        qint64 postCount;
        qint64 uptimeMsecs;
    };
public:
    typedef std::list<BoardInfo> BoardInfoList;
public:
    static const QString defaultFileTypes;
private:
    static QMap<QString, AbstractBoard *> boards;
    static bool boardsInitialized;
    static QReadWriteLock boardsLock;
    static bool globalCaptchaQuotaModified;
    static QMutex globalCaptchaQuotaMutex;
private:
    QMap<QString, unsigned int> captchaQuotaMap;
    mutable QMutex captchaQuotaMutex;
    qint64 postCount;
    mutable QMutex speedMutex;
    qint64 uptime;
    QElapsedTimer uptimeTimer;
public:
    explicit AbstractBoard();
    virtual ~AbstractBoard();
public:
    static LockingWrapper board(const QString &name);
    static BoardInfoList boardInfos(const QLocale &l, bool includeHidden = true);
    static QStringList boardNames(bool includeHidden = true);
    static bool isCaptchaQuotaModified();
    static void reloadBoards();
    static void restoreCaptchaQuota(const QByteArray &data);
    static QByteArray saveCaptchaQuota();
public:
    virtual void addFile(cppcms::application &app);
    unsigned int archiveLimit() const;
    QString bannerFileName() const;
    virtual bool beforeStoringEditedPost(const cppcms::http::request &req, cppcms::json::value &userData, Post &p,
                                         Thread &thread, QString *error = 0);
    virtual bool beforeStoringNewPost(const cppcms::http::request &req, Post *post,
                                      const Tools::PostParameters &params, bool thread, QString *error = 0,
                                      QString *description = 0);
    unsigned int bumpLimit() const;
    unsigned int captchaQuota() const;
    unsigned int captchaQuota(const QString &ip) const;
    unsigned int captchaQuota(const cppcms::http::request &req) const;
    void captchaSolved(const QString &ip);
    void captchaUsed(const QString &ip);
    virtual void createPost(cppcms::application &app);
    virtual void createThread(cppcms::application &app);
    virtual QString defaultUserName(const QLocale &l) const;
    bool draftsEnabled() const;
    virtual cppcms::json::value editedPostUserData(const Tools::PostParameters &params) const;
    virtual void handleBoard(cppcms::application &app, unsigned int page = 0);
    virtual void handleCatalog(cppcms::application &app);
    virtual void handleEditPost(cppcms::application &app, quint64 postNumber);
    virtual void handleRules(cppcms::application &app);
    virtual void handleThread(cppcms::application &app, quint64 threadNumber);
    bool isCaptchaEngineSupported(const QString &id) const;
    bool isEnabled() const;
    bool isFileTypeSupported(const QString &mimeType) const;
    bool isFileTypeSupported(const QByteArray &data) const;
    virtual bool isHidden() const;
    virtual MarkupElements markupElements() const;
    virtual QString name() const = 0;
    virtual QStringList postformRules(const QLocale &l) const;
    virtual bool postingEnabled() const;
    PostingSpeed postingSpeed() const;
    unsigned int postLimit() const;
    virtual QStringList rules(const QLocale &l) const;
    virtual bool saveFile(const Tools::File &f, FileTransaction &ft);
    virtual bool showWhois() const;
    virtual QString supportedCaptchaEngines() const;
    virtual QString supportedFileTypes() const;
    virtual bool testAddFileParams(const Tools::PostParameters &params, const Tools::FileList &files, const QLocale &l,
                                   QString *error = 0) const;
    virtual bool testParams(const Tools::PostParameters &params, const Tools::FileList &files, bool post,
                            const QLocale &l, QString *error = 0) const;
    unsigned int threadLimit() const;
    unsigned int threadsPerPage() const;
    virtual QString title(const QLocale &l) const = 0;
    virtual Content::Post toController(const Post &post, const cppcms::http::request &req, bool *ok = 0,
                                       QString *error = 0) const;
    virtual cppcms::json::object toJson(const Content::Post &post, const cppcms::http::request &req) const;
protected:
    virtual void beforeRenderBoard(const cppcms::http::request &req, Content::Board *c);
    virtual void beforeRenderCatalog(const cppcms::http::request &req, Content::Catalog *c);
    virtual void beforeRenderEditPost(const cppcms::http::request &req, Content::EditPost *c,
                                      const Content::Post &post);
    virtual void beforeRenderThread(const cppcms::http::request &req, Content::Thread *c);
    virtual Content::Board *createBoardController(const cppcms::http::request &req, QString &viewName);
    virtual Content::Catalog *createCatalogController(const cppcms::http::request &req, QString &viewName);
    virtual Content::EditPost *createEditPostController(const cppcms::http::request &req, QString &viewName);
    virtual Content::Thread *createThreadController(const cppcms::http::request &req, QString &viewName);
private:
    static void cleanupBoards();
    static QImage generateRandomImage(const QByteArray &hash, const QString &mimeType);
private:
    QStringList rulesImplementation(const QLocale &l, const QString &type) const;
private:
    friend class LockingWrapper;
};

#endif // ABSTRACTBOARD_H
