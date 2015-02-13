#ifndef SETTINGSLOCKER_H
#define SETTINGSLOCKER_H

class QSettings;

#include "global.h"

#include <QMutex>

class OLOLORD_EXPORT SettingsLocker
{
private:
    static QMutex mutex;
private:
    QSettings * const S;
public:
    explicit SettingsLocker();
    ~SettingsLocker();
public:
    QSettings *operator ->() const;
};

#endif // SETTINGSLOCKER_H
