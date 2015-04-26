#ifndef ABSTRACTCAPTCHAENGINE_H
#define ABSTRACTCAPTCHAENGINE_H

class QLocale;
class QStringList;

namespace cppcms
{

namespace http
{

class request;

}

}

#include "../global.h"
#include "../tools.h"

#include <QList>
#include <QMap>
#include <QReadLocker>
#include <QReadWriteLock>
#include <QSharedPointer>
#include <QString>

class OLOLORD_EXPORT AbstractCaptchaEngine
{
public:
    struct EngineInfo
    {
        std::string id;
        std::string title;
    };
    class OLOLORD_EXPORT LockingWrapper
    {
    private:
        AbstractCaptchaEngine * const Engine;
    private:
        QSharedPointer<QReadLocker> locker;
    public:
        LockingWrapper(const LockingWrapper &other);
    private:
        explicit LockingWrapper(AbstractCaptchaEngine *engine);
    public:
        AbstractCaptchaEngine *data() const;
        bool isNull() const;
    public:
        LockingWrapper &operator =(const LockingWrapper &other);
        AbstractCaptchaEngine *operator ->() const;
        bool operator !() const;
        operator bool() const;
    private:
        friend class AbstractCaptchaEngine;
    };
public:
    typedef QList<EngineInfo> EngineInfoList;
private:
    static QMap<QString, AbstractCaptchaEngine *> engines;
    static bool enginesInitialized;
    static QReadWriteLock enginesLock;
public:
    explicit AbstractCaptchaEngine();
    virtual ~AbstractCaptchaEngine();
public:
    static LockingWrapper engine(const QString &id);
    static QStringList engineIds();
    static EngineInfoList engineInfos(const QLocale &l);
    static void reloadEngines();
public:
    virtual bool checkCaptcha(const cppcms::http::request &req, const Tools::PostParameters &params,
                              QString &error) const = 0;
    virtual QString headerHtml() const;
    virtual QString id() const = 0;
    QString privateKey() const;
    QString publicKey() const;
    virtual QString scriptSource() const;
    virtual QString title(const QLocale &l) const = 0;
    virtual QString widgetHtml() const = 0;
private:
    static void cleanupEngines();
};

#endif // ABSTRACTCAPTCHAENGINE_H
