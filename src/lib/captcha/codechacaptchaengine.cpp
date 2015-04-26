#include "codechacaptchaengine.h"

#include "tools.h"
#include "translator.h"

#include <QByteArray>
#include <QLocale>
#include <QRegExp>
#include <QString>

#include <cppcms/http_request.h>

#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>

#include <sstream>
#include <string>

CodechaCaptchaEngine::CodechaCaptchaEngine()
{
    //
}

bool CodechaCaptchaEngine::checkCaptcha(const cppcms::http::request &req, const Tools::PostParameters &params,
                                        QString &error) const
{
    QString challenge = params.value("codecha_challenge_field");
    QString response = params.value("codecha_response_field");
    TranslatorQt tq(req);
    if (challenge.isEmpty() || response.isEmpty())
        return bRet(&error, tq.translate("CodechaCaptchaEngine", "Captcha is empty", "error"), false);
    try {
        curlpp::Cleanup curlppCleanup;
        Q_UNUSED(curlppCleanup)
        curlpp::Easy request;
        request.setOpt(curlpp::options::Url("http://codecha.org/api/verify"));
        cURLpp::Forms formParts;
        formParts.push_back(new cURLpp::FormParts::Content("challenge", challenge.toUtf8().data()));
        formParts.push_back(new cURLpp::FormParts::Content("response", response.toUtf8().data()));
        formParts.push_back(new cURLpp::FormParts::Content("remoteip", Tools::userIp(req).toLatin1().data()));
        formParts.push_back(new cURLpp::FormParts::Content("privatekey", privateKey().toLatin1().data()));
        request.setOpt(new cURLpp::Options::HttpPost(formParts));
        std::ostringstream os;
        os << request;
        QString result = Tools::fromStd(os.str());
        result.remove(QRegExp("\r?\n+"));
        if (result.compare("true", Qt::CaseInsensitive))
            return bRet(&error, tq.translate("CodechaCaptchaEngine", "Captcha is incorrect", "error"), false);
        return true;
    } catch (curlpp::RuntimeError &e) {
        return bRet(&error, Tools::fromStd(e.what()), false);
    } catch(curlpp::LogicError &e) {
        return bRet(&error, Tools::fromStd(e.what()), false);
    }
}

QString CodechaCaptchaEngine::id() const
{
    return "codecha";
}

QString CodechaCaptchaEngine::title(const QLocale &l) const
{
    return TranslatorQt(l).translate("CodechaCaptchaEngine", "Codecha - programmers' CAPTCHA", "title");
}

QString CodechaCaptchaEngine::widgetHtml() const
{
    QString s = "<div id=\"captcha\" class=\"codechaContainer\">";
    s += "<script type=\"text/javascript\" src=\"//codecha.org/api/challenge?k=" + publicKey() + "\"></script>";
    s += "</div>";
    return s;
}
