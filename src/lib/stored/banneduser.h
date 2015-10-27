#ifndef BANNEDUSER_H
#define BANNEDUSER_H

class Ban;

#include "../global.h"

#include <QDateTime>
#include <QString>

#include <odb/core.hxx>
#include <odb/qt/lazy-ptr.hxx>

PRAGMA_DB(object table("bannedUsers"))
class OLOLORD_EXPORT BannedUser
{
public:
    typedef QList< QLazyWeakPointer<Ban> > Bans;
private:
    PRAGMA_DB(id auto)
    quint64 id_;
    PRAGMA_DB(index unique member(id_))
    PRAGMA_DB(not_null)
    QString ip_;
    PRAGMA_DB(index unique member(ip_))
    PRAGMA_DB(value_not_null value_type("INTEGER") inverse(bannedUser_))
    Bans bans_;
public:
    explicit BannedUser(const QString &ip);
private:
    explicit BannedUser();
public:
    quint64 id() const;
    QString ip() const;
    const Bans &bans() const;
    Bans &bans();
private:
    friend class odb::access;
};

PRAGMA_DB(object table("bans"))
class OLOLORD_EXPORT Ban
{
private:
    PRAGMA_DB(id auto)
    quint64 id_;
    PRAGMA_DB(index unique member(id_))
    PRAGMA_DB(not_null)
    QString board_;
    PRAGMA_DB(not_null)
    QDateTime dateTime_;
    QDateTime expirationDateTime_;
    PRAGMA_DB(not_null)
    int level_;
    QString reason_;
    PRAGMA_DB(null)
    quint64 postId_;
    PRAGMA_DB(not_null)
    QLazySharedPointer<BannedUser> bannedUser_;
    PRAGMA_DB(index member(bannedUser_))
public:
    explicit Ban(QSharedPointer<BannedUser> bannedUser, const QString &board, const QDateTime &dateTime,
                 quint64 postId = 0L);
private:
    explicit Ban();
public:
    quint64 id() const;
    QString board() const;
    QDateTime dateTime() const;
    QDateTime expirationDateTime() const;
    int level() const;
    QString reason() const;
    quint64 postId() const;
    void setDateTime(const QDateTime &dateTime);
    void setExpirationDateTime(const QDateTime &dateTime);
    void setLevel(int level);
    void setReason(const QString &reason);
    QLazySharedPointer<BannedUser> bannedUser() const;
private:
    friend class odb::access;
};

#endif // BANNEDUSER_H
