#ifndef GOOGLERECAPTCHACAPTCHAENGINE_H
#define GOOGLERECAPTCHACAPTCHAENGINE_H

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

class OLOLORD_EXPORT GoogleRecaptchaCaptchaEngine : public AbstractCaptchaEngine
{
public:
    explicit GoogleRecaptchaCaptchaEngine();
public:
    bool checkCaptcha(const cppcms::http::request &req, const Tools::PostParameters &params, QString &error) const;
    QString id() const;
    QString scriptSource(bool asceticMode = false) const;
    QString title(const QLocale &l) const;
    QString widgetHtml(const cppcms::http::request &req, bool asceticMode = false) const;
};

#endif // GOOGLERECAPTCHACAPTCHAENGINE_H
