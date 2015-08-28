#include "googlerecaptchav1captchaengine.h"

#include "settingslocker.h"
#include "tools.h"
#include "translator.h"

#include <QDebug>
#include <QLocale>
#include <QRegExp>
#include <QString>

#include <cppcms/http_request.h>

#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>

#include <sstream>
#include <string>

GoogleRecaptchaV1CaptchaEngine::GoogleRecaptchaV1CaptchaEngine()
{
    //
}

bool GoogleRecaptchaV1CaptchaEngine::checkCaptcha(
        const cppcms::http::request &req, const Tools::PostParameters &params, QString &error) const
{
    QString captcha = params.value("recaptcha_response_field");
    QString challenge = params.value("recaptcha_challenge_field");
    TranslatorQt tq(req);
    if (challenge.isEmpty()) {
        return bRet(&error, tq.translate("GoogleRecaptchaV1CaptchaEngine", "Captcha challenge is empty", "error"),
                    false);
    }
    if (captcha.isEmpty()) {
        return bRet(&error, tq.translate("GoogleRecaptchaV1CaptchaEngine", "Captcha response is empty", "error"),
                    false);
    }
    try {
        curlpp::Cleanup curlppCleanup;
        Q_UNUSED(curlppCleanup)
        QString url = "https://www.google.com/recaptcha/api/verify?privatekey=%1&remoteip=%2&challenge=%3&response=%4";
        url = url.arg(privateKey()).arg(Tools::userIp(req)).arg(challenge).arg(captcha.replace(" ", "%20"));
        curlpp::Easy request;
        request.setOpt(curlpp::options::Url(Tools::toStd(url)));
        std::ostringstream os;
        os << request;
        QString result = Tools::fromStd(os.str());
        if (!result.contains("true", Qt::CaseInsensitive)) {
            return bRet(&error, tq.translate("GoogleRecaptchaV1CaptchaEngine", "Captcha is incorrect", "error"),
                        false);
        }
        return true;
    } catch (curlpp::RuntimeError &e) {
        return bRet(&error, Tools::fromStd(e.what()), false);
    } catch(curlpp::LogicError &e) {
        return bRet(&error, Tools::fromStd(e.what()), false);
    }
}

QString GoogleRecaptchaV1CaptchaEngine::id() const
{
    return "google-recaptcha-v1";
}

QString GoogleRecaptchaV1CaptchaEngine::title(const QLocale &/*l*/) const
{
    return "Google reCAPTCHA v1";
}

QString GoogleRecaptchaV1CaptchaEngine::widgetHtml(const cppcms::http::request &/*req*/, bool /*asceticMode*/) const
{
    return "<div id=\"captcha\"><script type=\"text/javascript\" "
            "src=\"https://www.google.com/recaptcha/api/challenge?k=" + publicKey() + "\"></script></div>";
}
