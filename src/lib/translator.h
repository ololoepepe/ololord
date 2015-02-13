#ifndef TRANSLATOR_H
#define TRANSLATOR_H

class QString;

namespace cppcms
{

namespace http
{

class request;

}

}

#include "global.h"

#include <BCoreApplication>

#include <QLocale>

#include <string>

namespace Translator
{

class Std;

class OLOLORD_EXPORT Qt
{
private:
    QLocale mlocale;
public:
    explicit Qt(const QLocale &locale = BCoreApplication::locale());
    explicit Qt(const Std &t);
    explicit Qt(const cppcms::http::request &req);
public:
    QLocale locale() const;
    QString translate(const char *context, const char *sourceText, const char *disambiguation = 0, int n = -1) const;
};

class OLOLORD_EXPORT Std
{
private:
    QLocale mlocale;
public:
    explicit Std(const QLocale &locale = BCoreApplication::locale());
    explicit Std(const Qt &t);
    explicit Std(const cppcms::http::request &req);
public:
    QLocale locale() const;
    std::string translate(const char *context, const char *sourceText, const char *disambiguation = 0,
                          int n = -1) const;
};

OLOLORD_EXPORT void registerTranslator(const QString &name);
OLOLORD_EXPORT void unregisterTranslator(const QString &name);

}

typedef Translator::Qt TranslatorQt;
typedef Translator::Std TranslatorStd;

#endif // TRANSLATOR_H
