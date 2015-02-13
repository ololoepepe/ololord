#ifndef BANNEDUSER_H
#define BANNEDUSER_H

#include "../global.h"

#include <QDateTime>
#include <QString>

#include <odb/core.hxx>

PRAGMA_DB(object table("bannedUsers"))
class OLOLORD_EXPORT BannedUser
{
private:
    PRAGMA_DB(id auto)
    quint64 id_;
    PRAGMA_DB(not_null)
    QString board_;
    PRAGMA_DB(not_null)
    QString ip_;
    PRAGMA_DB(not_null)
    QDateTime dateTime_;
    QDateTime expirationDateTime_;
    PRAGMA_DB(not_null)
    int level_;
    QString reason_;
public:
    explicit BannedUser(const QString &board, const QString &ip, const QDateTime &dateTime);
private:
    explicit BannedUser();
public:
    quint64 id() const;
    QString board() const;
    QString ip() const;
    QDateTime dateTime() const;
    QDateTime expirationDateTime() const;
    int level() const;
    QString reason() const;
    void setDateTime(const QDateTime &dateTime);
    void setExpirationDateTime(const QDateTime &dateTime);
    void setLevel(int level);
    void setReason(const QString &reason);
private:
    friend class odb::access;
};

#endif // BANNEDUSER_H
