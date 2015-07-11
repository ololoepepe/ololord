#include "ololordapplication.h"

#include "board/abstractboard.h"
#include "database.h"
#include "search.h"
#include "tools.h"
#include "translator.h"

#include <BCoreApplication>
#include <BDirTools>
#include <BeQt>

#include <QElapsedTimer>
#include <QString>
#include <QTimerEvent>

OlolordApplication::OlolordApplication(int &argc, char **argv, const QString &applicationName,
                                       const QString &organizationName) :
    BCoreApplication(argc, argv, applicationName, organizationName)
{
    Translator::registerTranslator("ololord");
#if defined(OLOLORD_BUILTIN_RESOURCES)
    Q_INIT_RESOURCE(ololord_res);
    Q_INIT_RESOURCE(ololord_static);
    Q_INIT_RESOURCE(ololord_static_css);
    Q_INIT_RESOURCE(ololord_static_img);
    Q_INIT_RESOURCE(ololord_static_js);
    Q_INIT_RESOURCE(ololord_static_video);
#endif
    captchaQuotaTimerId = startTimer(10 * BeQt::Minute);
    outdatedTimerId = startTimer(BeQt::Hour);
    searchTimerId = startTimer(10 * BeQt::Minute);
    uptimeTimer.start();
}

OlolordApplication::OlolordApplication(int &argc, char **argv, const InitialSettings &s) :
    BCoreApplication(argc, argv, s)
{
    Translator::registerTranslator("ololord");
#if defined(OLOLORD_BUILTIN_RESOURCES)
    Q_INIT_RESOURCE(ololord_res);
    Q_INIT_RESOURCE(ololord_static);
    Q_INIT_RESOURCE(ololord_static_css);
    Q_INIT_RESOURCE(ololord_static_img);
    Q_INIT_RESOURCE(ololord_static_js);
    Q_INIT_RESOURCE(ololord_static_video);
#endif
    captchaQuotaTimerId = startTimer(10 * BeQt::Minute);
    outdatedTimerId = startTimer(BeQt::Hour);
    searchTimerId = startTimer(10 * BeQt::Minute);
    uptimeTimer.start();
}

OlolordApplication::~OlolordApplication()
{
#if defined(OLOLORD_BUILTIN_RESOURCES)
    Q_CLEANUP_RESOURCE(ololord_res);
    Q_CLEANUP_RESOURCE(ololord_static);
    Q_CLEANUP_RESOURCE(ololord_static_css);
    Q_CLEANUP_RESOURCE(ololord_static_img);
    Q_CLEANUP_RESOURCE(ololord_static_js);
    Q_CLEANUP_RESOURCE(ololord_static_video);
#endif
}

qint64 OlolordApplication::uptime() const
{
    return uptimeTimer.elapsed();
}

void OlolordApplication::timerEvent(QTimerEvent *e)
{
    if (!e)
        return;
    if (e->timerId() == outdatedTimerId)
        Database::checkOutdatedEntries();
    else if (e->timerId() == searchTimerId && Search::isModified())
        BDirTools::writeFile(Tools::searchIndexFile(), Search::saveIndex());
    else if (e->timerId() == captchaQuotaTimerId && AbstractBoard::isCaptchaQuotaModified())
        BDirTools::writeFile(Tools::captchaQuotaFile(), AbstractBoard::saveCaptchaQuota());
}
