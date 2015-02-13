#ifndef OLOLORDAPPLICATION_H
#define OLOLORDAPPLICATION_H

#include "global.h"

#include <BCoreApplication>

#include <QString>

#if !defined(oApp)
#   define oApp (static_cast<OlolordApplication *>(BApplicationBase::binstance()))
#endif
#if defined(bApp)
#   undef bApp
#endif
#define bApp (static_cast<OlolordApplication *>(BApplicationBase::binstance()))

class OLOLORD_EXPORT OlolordApplication : public BCoreApplication
{
    Q_OBJECT
public:
    explicit OlolordApplication(int &argc, char **argv, const QString &applicationName = QString(),
                                const QString &organizationName = QString());
    explicit OlolordApplication(int &argc, char **argv, const InitialSettings &s);
    ~OlolordApplication();
private:
    Q_DISABLE_COPY(OlolordApplication)
};

#endif // OLOLORDAPPLICATION_H
