#include "abstractyandexcaptchaengine.h"

#include "tools.h"
#include "translator.h"

#include <BDirTools>

#include <QDebug>
#include <QRegExp>
#include <QString>
#include <QUrl>

#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>

#include <sstream>
#include <string>

AbstractYandexCaptchaEngine::AbstractYandexCaptchaEngine()
{
    //
}

bool AbstractYandexCaptchaEngine::checkCaptcha(const cppcms::http::request &req, const Tools::PostParameters &params,
                                               QString &error) const
{
    QString id = params.value("yandexCaptchaId");
    QString challenge = params.value("yandexCaptchaChallenge");
    QString response = params.value("yandexCaptchaResponse");
    TranslatorQt tq(req);
    if (id.isEmpty())
        return bRet(&error, tq.translate("AbstractYandexCaptchaEngine", "Captcha ID is empty", "error"), false);
    if (challenge.isEmpty())
        return bRet(&error, tq.translate("AbstractYandexCaptchaEngine", "Captcha challenge is empty", "error"), false);
    if (response.isEmpty())
        return bRet(&error, tq.translate("AbstractYandexCaptchaEngine", "Captcha is empty", "error"), false);
    try {
        curlpp::Cleanup curlppCleanup;
        Q_UNUSED(curlppCleanup)
        QString url = "http://cleanweb-api.yandex.ru/1.0/check-captcha?key=" + QUrl::toPercentEncoding(privateKey())
                + "&id=" + QUrl::toPercentEncoding(id) + "&captcha=" + QUrl::toPercentEncoding(challenge)
                + "&value=" + QUrl::toPercentEncoding(response);
        curlpp::Easy request;
        request.setOpt(curlpp::options::Url(Tools::toStd(url)));
        std::ostringstream os;
        os << request;
        QString result = Tools::fromStd(os.str());
        if (QRegExp("<ok>.*</ok>").indexIn(result) < 0)
            return bRet(&error, tq.translate("AbstractYandexCaptchaEngine", "Captcha is incorrect", "error"), false);
        return true;
    } catch (curlpp::RuntimeError &e) {
        return bRet(&error, Tools::fromStd(e.what()), false);
    } catch(curlpp::LogicError &e) {
        return bRet(&error, Tools::fromStd(e.what()), false);
    }
}

QString AbstractYandexCaptchaEngine::headerHtml() const
{
    QString fn = BDirTools::findResource("res/yandex_captcha_script.js", BDirTools::GlobalOnly);
    return BDirTools::readTextFile(fn, "UTF-8").replace("%privateKey%", privateKey()).replace("%type%", type());
}

QString AbstractYandexCaptchaEngine::id() const
{
    return "yandex-captcha-" + type();
}

QString AbstractYandexCaptchaEngine::title(const QLocale &l) const
{
    QString s = TranslatorQt(l).translate("AbstractYandexCaptchaEngine", "Yandex captcha", "title") + " (";
    QString t = type();
    if ("elatm" == t)
        s += TranslatorQt(l).translate("AbstractYandexCaptchaEngine", "Latin", "title");
    else if ("estd" == t)
        s += TranslatorQt(l).translate("AbstractYandexCaptchaEngine", "digits", "title");
    else if ("rus" == t)
        s += TranslatorQt(l).translate("AbstractYandexCaptchaEngine", "Cyrillic", "title");
    s += ")";
    return s;
}

QString AbstractYandexCaptchaEngine::widgetHtml() const
{
    QString s = "<div id=\"captcha\">";
    s += "<div name=\"image\"></div>";
    s += "<input type=\"hidden\" name=\"yandexCaptchaId\" />";
    s += "<input type=\"hidden\" name=\"yandexCaptchaChallenge\" />";
    s += "<input type=\"text\" name=\"yandexCaptchaResponse\" size=\"22\" />";
    s += "</div>";
    return s;
}

YandexCaptchaElatmEngine::YandexCaptchaElatmEngine()
{
    //
}

QString YandexCaptchaElatmEngine::type() const
{
    return "elatm";
}

YandexCaptchaEstdEngine::YandexCaptchaEstdEngine()
{
    //
}

QString YandexCaptchaEstdEngine::type() const
{
    return "estd";
}

YandexCaptchaRusEngine::YandexCaptchaRusEngine()
{
    //
}

QString YandexCaptchaRusEngine::type() const
{
    return "rus";
}
