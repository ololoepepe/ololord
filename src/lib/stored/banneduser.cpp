#include "banneduser.h"

#include <QDateTime>
#include <QString>

BannedUser::BannedUser(const QString &board, const QString &ip, const QDateTime &dateTime)
{
    id_ = 0L;
    board_ = board;
    ip_ = ip;
    dateTime_ = dateTime.toUTC();
    level_ = 1;
}

BannedUser::BannedUser()
{
    //
}

quint64 BannedUser::id() const
{
    return id_;
}

QString BannedUser::board() const
{
    return board_;
}

QString BannedUser::ip() const
{
    return ip_;
}

QDateTime BannedUser::dateTime() const
{
    return dateTime_.toUTC();
}

QDateTime BannedUser::expirationDateTime() const
{
    return expirationDateTime_.toUTC();
}

int BannedUser::level() const
{
    return level_;
}

QString BannedUser::reason() const
{
    return reason_;
}

void BannedUser::setDateTime(const QDateTime &dateTime)
{
    dateTime_ = dateTime;
}

void BannedUser::setExpirationDateTime(const QDateTime &dateTime)
{
    expirationDateTime_ = dateTime;
}

void BannedUser::setLevel(int level)
{
    level_ = level;
}

void BannedUser::setReason(const QString &reason)
{
    reason_ = reason;
}
