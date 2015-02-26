#ifndef ABSTRACTBOARD_H
#define ABSTRACTBOARD_H

namespace Content
{

class Board;
class Thread;

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

#include <QMap>
#include <QMutex>
#include <QString>

#include <list>
#include <string>

class OLOLORD_EXPORT AbstractBoard
{
public:
    enum ParamType
    {
        EmailParam = 1,
        NameParam,
        SubjectParam,
        TextParam,
        PasswordParam
    };
public:
    struct BoardInfo
    {
        std::string name;
        std::string title;
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
    unsigned int bumpLimit() const;
    unsigned int captchaQuota() const;
    unsigned int captchaQuota(const QString &ip) const;
    void captchaSolved(const QString &ip);
    void captchaUsed(const QString &ip);
    virtual void createPost(cppcms::application &app);
    virtual void createThread(cppcms::application &app);
    virtual QString defaultUserName(const QLocale &l) const;
    virtual void deleteFiles(const QStringList &fileNames);
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
    virtual bool postingEnabled() const;
    unsigned int postLimit() const;
    virtual bool processCode() const;
    virtual QStringList rules(const QLocale &l) const;
    virtual QString saveFile(const Tools::File &f, bool *ok = 0);
    virtual bool showWhois() const;
    virtual QString supportedFileTypes() const;
    virtual bool testParam(ParamType t, const QString &param, bool post, const QLocale &l, QString *error = 0) const;
    unsigned int threadLimit() const;
    unsigned int threadsPerPage() const;
    virtual QString thumbFileName(const QString &fn, QString &size, int &sizeX, int &sizeY) const;
    virtual QString title(const QLocale &l) const = 0;
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
