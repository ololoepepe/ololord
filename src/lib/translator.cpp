#include "translator.h"

#include "cache.h"
#include "tools.h"

#include <BTranslator>

#include <QLocale>
#include <QMutex>
#include <QMutexLocker>
#include <QSet>
#include <QString>

#include <cppcms/http_request.h>

#include <string>

static QMutex translatorMutex(QMutex::Recursive);
static QSet<QString> translatorNames;

static QString translate(const char *context, const char *sourceText, const char *disambiguation, int n,
                         const QLocale &l)
{
    QString src = QString::fromLatin1(sourceText);
    QMutexLocker locker(&translatorMutex);
    foreach (const QString &name, translatorNames.values()) {
        BTranslator *t = Cache::translator(name, l);
        if (!t) {
            t = new BTranslator(l, name);
            if (t->load()) {
                if (!Cache::cacheTranslator(name, l, t)) {
                    delete t;
                    t = 0;
                }
            }
        }
        QString s = t ? t->translate(context, sourceText, disambiguation, n) : src;
        if (!s.isEmpty() && s != src)
            return s;
    }
    return src;
}

namespace Translator
{

Qt::Qt(const QLocale &locale)
{
    mlocale = locale;
}

Qt::Qt(const Std &t)
{
    mlocale = t.locale();
}

Qt::Qt(const cppcms::http::request &req)
{
    mlocale = Tools::locale(req);
}

QLocale Qt::locale() const
{
    return mlocale;
}

QString Qt::translate(const char *context, const char *sourceText, const char *disambiguation, int n) const
{
    return ::translate(context, sourceText, disambiguation, n, mlocale);
}

Std::Std(const QLocale &locale)
{
    mlocale = locale;
}

Std::Std(const Qt &t)
{
    mlocale = t.locale();
}

Std::Std(const cppcms::http::request &req)
{
    mlocale = Tools::locale(req);
}

QLocale Std::locale() const
{
    return mlocale;
}

std::string Std::translate(const char *context, const char *sourceText, const char *disambiguation, int n) const
{
    return Tools::toStd(::translate(context, sourceText, disambiguation, n, mlocale));
}

void registerTranslator(const QString &name)
{
    if (name.isEmpty())
        return;
    QMutexLocker (&::translatorMutex);
    translatorNames.insert(name);
}

void unregisterTranslator(const QString &name)
{
    if (name.isEmpty())
        return;
    QMutexLocker (&::translatorMutex);
    translatorNames.remove(name);
}

}
