#ifndef CODECHACAPTCHAENGINE_H
#define CODECHACAPTCHAENGINE_H

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

class OLOLORD_EXPORT CodechaCaptchaEngine : public AbstractCaptchaEngine
{
public:
    explicit CodechaCaptchaEngine();
public:
    bool checkCaptcha(const cppcms::http::request &req, const Tools::PostParameters &params, QString &error) const;
    QString id() const;
    QString title(const QLocale &l) const;
    QString widgetHtml(const cppcms::http::request &req, bool asceticMode = false) const;
};

#endif // CODECHACAPTCHAENGINE_H
