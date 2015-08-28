#ifndef GOOGLERECAPTCHAV1CAPTCHAENGINE_H
#define GOOGLERECAPTCHAV1CAPTCHAENGINE_H

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

class OLOLORD_EXPORT GoogleRecaptchaV1CaptchaEngine : public AbstractCaptchaEngine
{
public:
    explicit GoogleRecaptchaV1CaptchaEngine();
public:
    bool checkCaptcha(const cppcms::http::request &req, const Tools::PostParameters &params, QString &error) const;
    QString id() const;
    QString title(const QLocale &l) const;
    QString widgetHtml(const cppcms::http::request &req, bool asceticMode = false) const;
};

#endif // GOOGLERECAPTCHAV1CAPTCHAENGINE_H
