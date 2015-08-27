#include "abstractcaptchaengine.h"

#include "abstractyandexcaptchaengine.h"
#include "codechacaptchaengine.h"
#include "googlerecaptchacaptchaengine.h"
#include "plugin/global/captchaenginefactoryplugininterface.h"
#include "settingslocker.h"

#include <BCoreApplication>
#include <BPluginInterface>
#include <BPluginWrapper>
#include <BSettingsNode>
#include <BTerminal>
#include <BTranslation>

#include <QList>
#include <QMap>
#include <QReadLocker>
#include <QReadWriteLock>
#include <QSet>
#include <QSettings>
#include <QSharedPointer>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QWriteLocker>

QMap<QString, AbstractCaptchaEngine *> AbstractCaptchaEngine::engines;
bool AbstractCaptchaEngine::enginesInitialized = false;
QReadWriteLock AbstractCaptchaEngine::enginesLock(QReadWriteLock::Recursive);

AbstractCaptchaEngine::LockingWrapper::LockingWrapper(const LockingWrapper &other) :
    Engine(other.Engine)
{
    locker = other.locker;
}

AbstractCaptchaEngine::LockingWrapper::LockingWrapper(AbstractCaptchaEngine *engine) :
    Engine(engine)
{
    locker = QSharedPointer<QReadLocker>(new QReadLocker(&AbstractCaptchaEngine::enginesLock));
}

AbstractCaptchaEngine *AbstractCaptchaEngine::LockingWrapper::data() const
{
    return Engine;
}

bool AbstractCaptchaEngine::LockingWrapper::isNull() const
{
    return !Engine;
}

AbstractCaptchaEngine *AbstractCaptchaEngine::LockingWrapper::operator ->() const
{
    return Engine;
}

bool AbstractCaptchaEngine::LockingWrapper::operator !() const
{
    return !Engine;
}

AbstractCaptchaEngine::LockingWrapper::operator bool() const
{
    return Engine;
}

AbstractCaptchaEngine::AbstractCaptchaEngine()
{
    //
}

AbstractCaptchaEngine::~AbstractCaptchaEngine()
{
    //
}

AbstractCaptchaEngine::LockingWrapper AbstractCaptchaEngine::engine(const QString &id)
{
    QReadLocker locker(&enginesLock);
    AbstractCaptchaEngine *e = engines.value(id);
    return e ? LockingWrapper(e) : LockingWrapper(0);
}

AbstractCaptchaEngine::EngineInfoList AbstractCaptchaEngine::engineInfos(const QLocale &l)
{
    EngineInfoList list;
    QReadLocker locker(&enginesLock);
    foreach (const QString &key, engines.keys()) {
        AbstractCaptchaEngine *engine = engines.value(key);
        if (!engine)
            continue;
        EngineInfo info;
        info.id = Tools::toStd(engine->id());
        info.title = Tools::toStd(engine->title(l));
        list.push_back(info);
    }
    return list;
}

QStringList AbstractCaptchaEngine::engineIds()
{
    QReadLocker locker(&enginesLock);
    return QStringList(engines.keys());
}

void AbstractCaptchaEngine::reloadEngines()
{
    QWriteLocker locker(&enginesLock);
    QSet<QString> ids = QSet<QString>::fromList(engines.keys());
    BSettingsNode *n = BTerminal::rootSettingsNode()->find("Captcha");
    foreach (BSettingsNode *nn, n->childNodes()) {
        if (!ids.contains(nn->key()))
            continue;
        n->removeChild(nn);
        delete nn;
    }
    foreach (AbstractCaptchaEngine *e, engines.values())
        delete e;
    engines.clear();
    AbstractCaptchaEngine *e = new GoogleRecaptchaCaptchaEngine;
    engines.insert(e->id(), e);
    e = new CodechaCaptchaEngine;
    engines.insert(e->id(), e);
    e = new YandexCaptchaElatmEngine;
    engines.insert(e->id(), e);
    e = new YandexCaptchaEstdEngine;
    engines.insert(e->id(), e);
    e = new YandexCaptchaRusEngine;
    engines.insert(e->id(), e);
    foreach (BPluginWrapper *pw, BCoreApplication::pluginWrappers("captcha-engine-factory")) {
        pw->unload();
        BCoreApplication::removePlugin(pw);
    }
    BCoreApplication::loadPlugins(QStringList() << "captcha-engine-factory");
    foreach (BPluginWrapper *pw, BCoreApplication::pluginWrappers("captcha-engine-factory")) {
        CaptchaEngineFactoryPluginInterface *i = qobject_cast<CaptchaEngineFactoryPluginInterface *>(pw->instance());
        if (!i)
            continue;
        foreach (AbstractCaptchaEngine *e, i->createCaptchaEngines()) {
            if (!e)
                continue;
            QString s = e->id();
            if (s.isEmpty()) {
                delete e;
                continue;
            }
            if (engines.contains(s))
                delete engines.take(s);
            engines.insert(s, e);
        }
    }
    foreach (const QString &s, engines.keys()) {
        BSettingsNode *nn = new BSettingsNode(s, n);
        BSettingsNode *nnn = new BSettingsNode(QVariant::String, "private_key", nn);
        nnn->setDescription(BTranslation::translate("AbstractCaptchaEngine", "Private captcha key.\n"
                                                    "Is stored locally and does not appear anywhere in any HTML pages "
                                                    "or other resources."));
        nnn = new BSettingsNode(QVariant::String, "public_key", nn);
        nnn->setDescription(BTranslation::translate("AbstractCaptchaEngine", "Public captcha key.\n"
                                                    "Apperas in the HTML pages."));
    }
    if (!enginesInitialized)
        qAddPostRoutine(&cleanupEngines);
    enginesInitialized = true;
}

QString AbstractCaptchaEngine::headerHtml(bool /*asceticMode*/) const
{
    return "";
}

QString AbstractCaptchaEngine::privateKey() const
{
    QString s = id();
    if (s.isEmpty())
        return "";
    return SettingsLocker()->value("Captcha/" + s +"/private_key").toString();
}

QString AbstractCaptchaEngine::publicKey() const
{
    QString s = id();
    if (s.isEmpty())
        return "";
    return SettingsLocker()->value("Captcha/" + s +"/public_key").toString();
}

QString AbstractCaptchaEngine::scriptSource(bool /*asceticMode*/) const
{
    return "";
}

void AbstractCaptchaEngine::cleanupEngines()
{
    QWriteLocker locker(&enginesLock);
    foreach (AbstractCaptchaEngine *e, engines)
        delete e;
    engines.clear();
}
