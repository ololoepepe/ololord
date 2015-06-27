#ifndef ABSTRACTYANDEXCAPTCHAENGINE_H
#define ABSTRACTYANDEXCAPTCHAENGINE_H

class QLocale;

namespace cppcms
{

namespace http
{

class request;

}

}

#include "abstractcaptchaengine.h"

#include <QString>

class OLOLORD_EXPORT AbstractYandexCaptchaEngine : public AbstractCaptchaEngine
{
public:
    struct CaptchaInfo
    {
        QString challenge;
        QString url;
    };
protected:
    explicit AbstractYandexCaptchaEngine();
public:
    static CaptchaInfo getCaptchaInfo(const QString &type, const QLocale &l, bool *ok = 0, QString *error = 0);
public:
    bool checkCaptcha(const cppcms::http::request &req, const Tools::PostParameters &params, QString &error) const;
    QString headerHtml(bool asceticMode = false) const;
    QString id() const;
    QString title(const QLocale &l) const;
    QString widgetHtml(const cppcms::http::request &req, bool asceticMode = false) const;
protected:
    virtual QString type() const = 0;
};

class OLOLORD_EXPORT YandexCaptchaElatmEngine : public AbstractYandexCaptchaEngine
{
public:
    explicit YandexCaptchaElatmEngine();
protected:
    QString type() const;
};

class OLOLORD_EXPORT YandexCaptchaEstdEngine : public AbstractYandexCaptchaEngine
{
public:
    explicit YandexCaptchaEstdEngine();
protected:
    QString type() const;
};

class OLOLORD_EXPORT YandexCaptchaRusEngine : public AbstractYandexCaptchaEngine
{
public:
    explicit YandexCaptchaRusEngine();
protected:
    QString type() const;
};

#endif // ABSTRACTYANDEXCAPTCHAENGINE_H
