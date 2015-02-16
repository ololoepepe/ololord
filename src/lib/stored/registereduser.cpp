#include "registereduser.h"

#include <QByteArray>
#include <QDateTime>
#include <QString>

RegisteredUser::RegisteredUser(const QByteArray &hashpass, const QDateTime &dateTime, Level level)
{
    hashpass_ = hashpass;
    dateTime_ = dateTime.toUTC();
    level_ = level;
}

RegisteredUser::RegisteredUser()
{
    //
}

quint64 RegisteredUser::id() const
{
    return id_;
}

QByteArray RegisteredUser::hashpass() const
{
    return hashpass_;
}

QDateTime RegisteredUser::dateTime() const
{
    return QDateTime(dateTime_.date(), dateTime_.time(), Qt::UTC);
}

int RegisteredUser::level() const
{
    return level_;
}

void RegisteredUser::setHashpass(const QByteArray &hashpass)
{
    hashpass_ = hashpass;
}

void RegisteredUser::setLevel(Level level)
{
    level_ = level;
}
