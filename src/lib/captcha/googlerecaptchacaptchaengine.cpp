#include "googlerecaptchacaptchaengine.h"

#include "settingslocker.h"
#include "tools.h"
#include "translator.h"

#include <QLocale>
#include <QRegExp>
#include <QString>

#include <cppcms/http_request.h>

#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>

#include <sstream>
#include <string>

GoogleRecaptchaCaptchaEngine::GoogleRecaptchaCaptchaEngine()
{
    //
}

bool GoogleRecaptchaCaptchaEngine::checkCaptcha(const cppcms::http::request &req, const Tools::PostParameters &params,
                                                QString &error) const
{
    QString captcha = params.value("g-recaptcha-response");
    TranslatorQt tq(req);
    if (captcha.isEmpty())
        return bRet(&error, tq.translate("GoogleRecaptchaCaptchaEngine", "Captcha is empty", "error"), false);
    try {
        curlpp::Cleanup curlppCleanup;
        Q_UNUSED(curlppCleanup)
        QString url = "https://www.google.com/recaptcha/api/siteverify?secret=%1&response=%2&remoteip=%3";
        url = url.arg(privateKey()).arg(captcha);
        url = url.arg(Tools::userIp(req));
        curlpp::Easy request;
        request.setOpt(curlpp::options::Url(Tools::toStd(url)));
        std::ostringstream os;
        os << request;
        QString result = Tools::fromStd(os.str());
        result.remove(QRegExp(".*\"success\":\\s*\"?"));
        result.remove(QRegExp("\"?\\,?\\s+.+"));
        if (result.compare("true", Qt::CaseInsensitive))
            return bRet(&error, tq.translate("GoogleRecaptchaCaptchaEngine", "Captcha is incorrect", "error"), false);
        return true;
    } catch (curlpp::RuntimeError &e) {
        return bRet(&error, Tools::fromStd(e.what()), false);
    } catch(curlpp::LogicError &e) {
        return bRet(&error, Tools::fromStd(e.what()), false);
    }
}

QString GoogleRecaptchaCaptchaEngine::id() const
{
    return "google-recaptcha";
}

QString GoogleRecaptchaCaptchaEngine::scriptSource(bool /*asceticMode*/) const
{
    return "https://www.google.com/recaptcha/api.js";
}

QString GoogleRecaptchaCaptchaEngine::title(const QLocale &/*l*/) const
{
    return "Google reCAPTCHA";
}

QString GoogleRecaptchaCaptchaEngine::widgetHtml(const cppcms::http::request &/*req*/, bool /*asceticMode*/) const
{
    return "<div id=\"captcha\" class=\"g-recaptcha\" data-sitekey=\"" + publicKey() + "\"></div>";
}
