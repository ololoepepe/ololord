#ifndef ABSTRACTYANDEXCAPTCHAENGINE_H
#define ABSTRACTYANDEXCAPTCHAENGINE_H

class QLocale;
class QString;

namespace cppcms
{

namespace http
{

class request;

}

}

#include "abstractcaptchaengine.h"

class OLOLORD_EXPORT AbstractYandexCaptchaEngine : public AbstractCaptchaEngine
{
protected:
    explicit AbstractYandexCaptchaEngine();
public:
    bool checkCaptcha(const cppcms::http::request &req, const Tools::PostParameters &params, QString &error) const;
    QString headerHtml() const;
    QString id() const;
    QString title(const QLocale &l) const;
    QString widgetHtml() const;
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
