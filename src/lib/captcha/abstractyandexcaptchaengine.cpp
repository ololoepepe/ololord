#include "abstractyandexcaptchaengine.h"

#include "settingslocker.h"
#include "tools.h"
#include "translator.h"

#include <BDirTools>

#include <QDebug>
#include <QRegExp>
#include <QSettings>
#include <QString>
#include <QUrl>
#include <QVariant>

#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>

#include <sstream>
#include <string>

AbstractYandexCaptchaEngine::AbstractYandexCaptchaEngine()
{
    //
}

AbstractYandexCaptchaEngine::CaptchaInfo AbstractYandexCaptchaEngine::getCaptchaInfo(
        const QString &type, const QLocale &l, bool *ok, QString *error)
{
    AbstractCaptchaEngine::LockingWrapper e = AbstractCaptchaEngine::engine("yandex-captcha-" + type);
    TranslatorQt tq(l);
    if (e.isNull()) {
        return bRet(ok, false, error, tq.translate("AbstractYandexCaptchaEngine",
                                                   "No engine for this captcha type", "error"), CaptchaInfo());
    }
    try {
        curlpp::Cleanup curlppCleanup;
        Q_UNUSED(curlppCleanup)
        QString url = "http://cleanweb-api.yandex.ru/1.0/get-captcha?key="
                + QUrl::toPercentEncoding(e->privateKey()) + "&type=" + QUrl::toPercentEncoding(type);
        curlpp::Easy request;
        request.setOpt(curlpp::options::Url(Tools::toStd(url)));
        std::ostringstream os;
        os << request;
        QString result = Tools::fromStd(os.str());
        QRegExp rxc("<captcha>.+</captcha>");
        QRegExp rxu("<url>.+</url>");
        if (rxc.indexIn(result) < 0 || rxu.indexIn(result) < 0) {
            return bRet(ok, false, error, tq.translate("AbstractYandexCaptchaEngine", "Yandex captcha service error",
                                                       "error"), CaptchaInfo());
        }
        QString challenge = rxc.cap().remove("<captcha>").remove("</captcha>");
        QString iurl = rxu.cap().remove("<url>").remove("</url>");
        CaptchaInfo inf;
        inf.challenge = challenge;
        inf.url = iurl;
        return bRet(ok, true, error, QString(), inf);
    } catch (curlpp::RuntimeError &e) {
        return bRet(ok, false, error, QString(e.what()), CaptchaInfo());
    } catch(curlpp::LogicError &e) {
        return bRet(ok, false, error, QString(e.what()), CaptchaInfo());
    }
}

bool AbstractYandexCaptchaEngine::checkCaptcha(const cppcms::http::request &req, const Tools::PostParameters &params,
                                               QString &error) const
{
    QString challenge = params.value("yandexCaptchaChallenge");
    QString response = params.value("yandexCaptchaResponse");
    TranslatorQt tq(req);
    if (challenge.isEmpty())
        return bRet(&error, tq.translate("AbstractYandexCaptchaEngine", "Captcha challenge is empty", "error"), false);
    if (response.isEmpty())
        return bRet(&error, tq.translate("AbstractYandexCaptchaEngine", "Captcha is empty", "error"), false);
    try {
        curlpp::Cleanup curlppCleanup;
        Q_UNUSED(curlppCleanup)
        QString url = "http://cleanweb-api.yandex.ru/1.0/check-captcha?key=" + QUrl::toPercentEncoding(privateKey())
                + "&captcha=" + QUrl::toPercentEncoding(challenge) + "&value=" + QUrl::toPercentEncoding(response);
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

QString AbstractYandexCaptchaEngine::headerHtml(bool asceticMode) const
{
    if (asceticMode)
        return "";
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

QString AbstractYandexCaptchaEngine::widgetHtml(const cppcms::http::request &req, bool asceticMode) const
{
    QString s = "<div id=\"captcha\">";
    if (asceticMode) {
        s += "<div name=\"image\"></div>";
        QString type = Tools::cookieValue(req, "captchaEngine").split('-').last();
        bool ok = false;
        QString err;
        CaptchaInfo inf = getCaptchaInfo(type, Tools::locale(req), &ok, &err);
        if (!ok) {
            s += "<div name=\"image\"><img src=\"/" + SettingsLocker()->value("Site/path_prefix").toString()
                    + "img/yandex-hernya.png\" title=\"" + err + "\"></div>";
        } else {
            s += "<div name=\"image\"><img src=\"" + inf.url.replace("https://", "http://") + "\"></div>";
            s += "<input type=\"hidden\" name=\"yandexCaptchaChallenge\" value=\"" + inf.challenge + "\" />";
        }
    } else {
        s += "<div name=\"image\"></div>";
        s += "<input type=\"hidden\" name=\"yandexCaptchaChallenge\" />";
    }
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
