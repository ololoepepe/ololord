#include "banneduser.h"

#include <QDateTime>
#include <QString>

BannedUser::BannedUser(const QString &board, const QString &ip, const QDateTime &dateTime, quint64 postId)
{
    id_ = 0L;
    board_ = board;
    ip_ = ip;
    dateTime_ = dateTime.toUTC();
    level_ = 1;
    postId_ = postId;
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
    return QDateTime(dateTime_.date(), dateTime_.time(), Qt::UTC);
}

QDateTime BannedUser::expirationDateTime() const
{
    return QDateTime(expirationDateTime_.date(), expirationDateTime_.time(), Qt::UTC);
}

int BannedUser::level() const
{
    return level_;
}

QString BannedUser::reason() const
{
    return reason_;
}

quint64 BannedUser::postId() const
{
    return postId_;
}

void BannedUser::setDateTime(const QDateTime &dateTime)
{
    dateTime_ = dateTime.toUTC();
}

void BannedUser::setExpirationDateTime(const QDateTime &dateTime)
{
    expirationDateTime_ = dateTime.toUTC();
}

void BannedUser::setLevel(int level)
{
    level_ = level;
}

void BannedUser::setReason(const QString &reason)
{
    reason_ = reason;
}
