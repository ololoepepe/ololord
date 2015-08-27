#include "mailrunocaptchaengine.h"

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

MailruNocaptchaEngine::MailruNocaptchaEngine()
{
    //
}

bool MailruNocaptchaEngine::checkCaptcha(const cppcms::http::request &req, const Tools::PostParameters &params,
                                         QString &error) const
{
    QString captchaId = params.value("captcha_id");
    QString captchaValue = params.value("captcha_value");
    TranslatorQt tq(req);
    if (captchaId.isEmpty())
        return bRet(&error, tq.translate("MailruNocaptchaEngine", "Captcha ID is empty", "error"), false);
    if (captchaValue.isEmpty())
        return bRet(&error, tq.translate("MailruNocaptchaEngine", "Captcha value is empty", "error"), false);
    try {
        curlpp::Cleanup curlppCleanup;
        Q_UNUSED(curlppCleanup)
        QString url = "https://api-nocaptcha.mail.ru/check?private_key=%1&captcha_id=%2&captcha_value=%3";
        url = url.arg(privateKey()).arg(captchaId).arg(captchaValue);
        curlpp::Easy request;
        request.setOpt(curlpp::options::Url(Tools::toStd(url)));
        std::ostringstream os;
        os << request;
        QString result = Tools::fromStd(os.str());
        if (result.contains("is_correct", Qt::CaseInsensitive)) {
            if (!result.contains("true", Qt::CaseInsensitive))
                return bRet(&error, tq.translate("MailruNocaptchaEngine", "Captcha is incorrect", "error"), false);
        } else {
            return bRet(&error, tq.translate("MailruNocaptchaEngine", "Internal error", "error"), false);
        }
        return true;
    } catch (curlpp::RuntimeError &e) {
        return bRet(&error, Tools::fromStd(e.what()), false);
    } catch(curlpp::LogicError &e) {
        return bRet(&error, Tools::fromStd(e.what()), false);
    }
}

QString MailruNocaptchaEngine::id() const
{
    return "mailru-nocaptcha";
}

QString MailruNocaptchaEngine::scriptSource(bool /*asceticMode*/) const
{
    return "https://api-nocaptcha.mail.ru/captcha?public_key=" + publicKey();
}

QString MailruNocaptchaEngine::title(const QLocale &l) const
{
    return TranslatorQt(l).translate("MailruNocaptchaEngine", "NOCAPTCHA@MAIL.RU", "title");
}

QString MailruNocaptchaEngine::widgetHtml(const cppcms::http::request &/*req*/, bool /*asceticMode*/) const
{
    return "<div id=\"captcha\"><div id=\"nocaptcha\"></div></div>";
}
