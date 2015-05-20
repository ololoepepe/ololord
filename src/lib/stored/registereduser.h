#ifndef REGISTEREDUSER_H
#define REGISTEREDUSER_H

#include "../global.h"

#include <QByteArray>
#include <QDateTime>
#include <QString>
#include <QStringList>

#include <odb/core.hxx>

PRAGMA_DB(object table("registeredUsers"))
class OLOLORD_EXPORT RegisteredUser
{
public:
    enum Level
    {
        NoLevel = 0,
        UserLevel = 1,
        ModerLevel = 10,
        AdminLevel = 100
    };
private:
    PRAGMA_DB(id auto)
    quint64 id_;
    QByteArray hashpass_;
    PRAGMA_DB(index unique member(hashpass_))
    PRAGMA_DB(not_null)
    QDateTime dateTime_;
    PRAGMA_DB(not_null)
    int level_;
    QByteArray boards_;
public:
    explicit RegisteredUser(const QByteArray &hashpass, const QDateTime &dateTime,
                            Level level = UserLevel, const QStringList &boards = QStringList("*"));
private:
    explicit RegisteredUser();
public:
    quint64 id() const;
    QByteArray hashpass() const;
    QDateTime dateTime() const;
    int level() const;
    QStringList boards() const;
    void setHashpass(const QByteArray &hashpass);
    void setLevel(Level level);
    void setBoards(const QStringList &boards);
private:
    friend class odb::access;
};

PRAGMA_DB(view object(RegisteredUser))
struct OLOLORD_EXPORT RegisteredUserLevel
{
    int level;
};

#endif // REGISTEREDUSER_H
