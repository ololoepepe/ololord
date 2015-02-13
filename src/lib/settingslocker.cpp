#include "settingslocker.h"

#include <BCoreApplication>

#include <QMutex>

QMutex SettingsLocker::mutex(QMutex::Recursive);

SettingsLocker::SettingsLocker() :
    S(bSettings)
{
    mutex.lock();
}


SettingsLocker::~SettingsLocker()
{
    mutex.unlock();
}

QSettings *SettingsLocker::operator ->() const
{
    return S;
}
