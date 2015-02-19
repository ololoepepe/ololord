#include "prboard.h"

#include "controller/board.h"
#include "controller/prboard.h"
#include "controller/prthread.h"
#include "controller/thread.h"
#include "settingslocker.h"
#include "tools.h"
#include "translator.h"

#include <QByteArray>
#include <QDebug>
#include <QLocale>
#include <QMap>
#include <QMutex>
#include <QRegExp>
#include <QSettings>
#include <QString>
#include <QVariant>

#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>

#include <list>
#include <string>
#include <sstream>

prBoard::prBoard()
{
    //
}

bool prBoard::isCaptchaValid(const cppcms::http::request &req, const Tools::PostParameters &params,
                             QString &error) const
{
    QString challenge = params.value("codecha_challenge_field");
    QString response = params.value("codecha_response_field");
    TranslatorQt tq(req);
    if (challenge.isEmpty() || response.isEmpty())
        return bRet(&error, tq.translate("prBoard", "Captcha is empty", "error"), false);
    try {
        curlpp::Cleanup curlppCleanup;
        Q_UNUSED(curlppCleanup)
        curlpp::Easy request;
        request.setOpt(curlpp::options::Url("http://codecha.org/api/verify"));
        cURLpp::Forms formParts;
        formParts.push_back(new cURLpp::FormParts::Content("challenge", challenge.toUtf8().data()));
        formParts.push_back(new cURLpp::FormParts::Content("response", response.toUtf8().data()));
        formParts.push_back(new cURLpp::FormParts::Content("remoteip",  Tools::userIp(req).toLatin1().data()));
        QString privateKey = SettingsLocker()->value("Site/codecha_private_key").toString();
        formParts.push_back(new cURLpp::FormParts::Content("privatekey", privateKey.toLatin1().data()));
        request.setOpt(new cURLpp::Options::HttpPost(formParts));
        std::ostringstream os;
        os << request;
        QString result = Tools::fromStd(os.str());
        result.remove(QRegExp("\r?\n+"));
        if (result.compare("true", Qt::CaseInsensitive))
            return bRet(&error, tq.translate("prBoard", "Captcha is incorrect", "error"), false);
        return true;
    } catch (curlpp::RuntimeError &e) {
        return bRet(&error, Tools::fromStd(e.what()), false);
    } catch(curlpp::LogicError &e) {
        return bRet(&error, Tools::fromStd(e.what()), false);
    }
}

QString prBoard::name() const
{
    return "pr";
}

bool prBoard::processCode() const
{
    return true;
}

QString prBoard::title(const QLocale &l) const
{
    TranslatorQt tq(l);
    return tq.translate("prBoard", "/pr/ogramming", "board title");
}

void prBoard::beforeRenderBoard(const cppcms::http::request &/*req*/, Content::Board *c)
{
    Content::prBoard *cc = dynamic_cast<Content::prBoard *>(c);
    if (!cc)
        return;
    cc->codechaPublicKey = Tools::toStd(SettingsLocker()->value("Site/codecha_public_key").toString());
}

void prBoard::beforeRenderThread(const cppcms::http::request &/*req*/, Content::Thread *c)
{
    Content::prThread *cc = dynamic_cast<Content::prThread *>(c);
    if (!cc)
        return;
    cc->codechaPublicKey = Tools::toStd(SettingsLocker()->value("Site/codecha_public_key").toString());
}

Content::Board *prBoard::createBoardController(const cppcms::http::request &/*req*/, QString &viewName)
{
    viewName = "pr_board";
    return new Content::prBoard;
}

Content::Thread *prBoard::createThreadController(const cppcms::http::request &/*req*/, QString &viewName)
{
    viewName = "pr_thread";
    return new Content::prThread;
}
