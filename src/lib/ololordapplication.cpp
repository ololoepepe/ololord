#include "ololordapplication.h"

#include "database.h"
#include "translator.h"

#include <BCoreApplication>
#include <BeQt>

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
    timerId = startTimer(BeQt::Hour);
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
    timerId = startTimer(BeQt::Hour);
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

void OlolordApplication::timerEvent(QTimerEvent *e)
{
    if (!e || e->timerId() != timerId)
        return;
   Database::checkOutdatedEntries();
}
