#include "banneduser.h"

#include <QDateTime>
#include <QString>

BannedUser::BannedUser(const QString &ip)
{
    id_ = 0L;
    ip_ = ip;
}

BannedUser::BannedUser()
{
    //
}

quint64 BannedUser::id() const
{
    return id_;
}

QString BannedUser::ip() const
{
    return ip_;
}

const BannedUser::Bans &BannedUser::bans() const
{
    return bans_;
}

BannedUser::Bans &BannedUser::bans()
{
    return bans_;
}

Ban::Ban(QSharedPointer<BannedUser> bannedUser, const QString &board, const QDateTime &dateTime, quint64 postId)
{
    id_ = 0L;
    bannedUser_ = bannedUser;
    board_ = board;
    dateTime_ = dateTime.toUTC();
    level_ = 1;
    postId_ = postId;
}

Ban::Ban()
{
    //
}

quint64 Ban::id() const
{
    return id_;
}

QString Ban::board() const
{
    return board_;
}

QDateTime Ban::dateTime() const
{
    return QDateTime(dateTime_.date(), dateTime_.time(), Qt::UTC);
}

QDateTime Ban::expirationDateTime() const
{
    return QDateTime(expirationDateTime_.date(), expirationDateTime_.time(), Qt::UTC);
}

int Ban::level() const
{
    return level_;
}

QString Ban::reason() const
{
    return reason_;
}

quint64 Ban::postId() const
{
    return postId_;
}

void Ban::setDateTime(const QDateTime &dateTime)
{
    dateTime_ = dateTime.toUTC();
}

void Ban::setExpirationDateTime(const QDateTime &dateTime)
{
    expirationDateTime_ = dateTime.toUTC();
}

void Ban::setLevel(int level)
{
    level_ = level;
}

void Ban::setReason(const QString &reason)
{
    reason_ = reason;
}

QLazySharedPointer<BannedUser> Ban::bannedUser() const
{
    return bannedUser_;
}
