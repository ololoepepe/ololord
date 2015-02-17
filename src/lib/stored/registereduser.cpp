#include "registereduser.h"

#include <BeQt>

#include <QByteArray>
#include <QDateTime>
#include <QString>
#include <QStringList>
#include <QVariant>

RegisteredUser::RegisteredUser(const QByteArray &hashpass, const QDateTime &dateTime, Level level,
                               const QStringList &boards)
{
    hashpass_ = hashpass;
    dateTime_ = dateTime.toUTC();
    level_ = level;
    setBoards(boards);
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

QStringList RegisteredUser::boards() const
{
    return BeQt::deserialize(boards_).toStringList();
}

void RegisteredUser::setHashpass(const QByteArray &hashpass)
{
    hashpass_ = hashpass;
}

void RegisteredUser::setLevel(Level level)
{
    level_ = level;
}

void RegisteredUser::setBoards(const QStringList &boards)
{
    boards_ = BeQt::serialize(boards);
}
