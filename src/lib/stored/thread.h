#ifndef THREAD_H
#define THREAD_H

class Post;

#include "../global.h"

#include <QByteArray>
#include <QDateTime>
#include <QList>
#include <QSharedPointer>
#include <QString>
#include <QStringList>

#include <odb/core.hxx>
#include <odb/qt/lazy-ptr.hxx>

PRAGMA_DB(object table("threads"))
class OLOLORD_EXPORT Thread
{
public:
    typedef QList< QLazyWeakPointer<Post> > Posts;
private:
    PRAGMA_DB(id auto)
    quint64 id_;
    PRAGMA_DB(not_null)
    QString board_;
    PRAGMA_DB(not_null)
    quint64 number_;
    PRAGMA_DB(not_null)
    QDateTime dateTime_;
    bool archived_;
    bool fixed_;
    bool postingEnabled_;
    PRAGMA_DB(value_not_null value_type("INTEGER") inverse(thread_))
    Posts posts_;
public:
    explicit Thread(const QString &board, quint64 number, const QDateTime &dateTime);
private:
    explicit Thread();
public:
    quint64 id() const;
    QString board() const;
    quint64 number() const;
    QDateTime dateTime() const;
    bool archived() const;
    bool fixed() const;
    bool postingEnabled() const;
    const Posts &posts() const;
    Posts &posts();
    void setArchived(bool archived);
    void setBoard(const QString &board);
    void setDateTime(const QDateTime &dateTime);
    void setFixed(bool fixed);
    void setPostingEnabled(bool enabled);
private:
    friend class odb::access;
};

PRAGMA_DB(view object(Thread))
struct OLOLORD_EXPORT ThreadCount
{
    PRAGMA_DB(column("count(" + Thread::id_ + ")"))
    int count;
};

PRAGMA_DB(view object(Thread))
struct OLOLORD_EXPORT ThreadIdDateTimeFixed
{
    quint64 id;
    quint64 number;
    QDateTime dateTime;
    bool fixed;
};

PRAGMA_DB(object table("posts"))
class OLOLORD_EXPORT Post
{
private:
    PRAGMA_DB(id auto)
    quint64 id_;
    PRAGMA_DB(not_null)
    QString board_;
    PRAGMA_DB(not_null)
    quint64 number_;
    PRAGMA_DB(not_null)
    QDateTime dateTime_;
    bool bannedFor_;
    QString email_;
    QByteArray files_;
    QByteArray hashpass_;
    QString name_;
    PRAGMA_DB(not_null)
    QByteArray password_;
    QString posterIp_;
    QString subject_;
    QString text_;
    PRAGMA_DB(not_null)
    QLazySharedPointer<Thread> thread_;
public:
    explicit Post(const QString &board, quint64 number, const QDateTime &dateTime, QSharedPointer<Thread> thread,
                  const QString &posterIp, const QByteArray &password, const QByteArray &hashpass = QByteArray());
public:
    explicit Post();
public:
    quint64 id() const;
    QString board() const;
    quint64 number() const;
    QDateTime dateTime() const;
    bool bannedFor() const;
    QString email() const;
    QStringList files() const;
    QByteArray hashpass() const;
    QString name() const;
    QByteArray password() const;
    QString posterIp() const;
    void setBannedFor(bool banned);
    void setEmail(const QString &email);
    void setFiles(const QStringList &files);
    void setName(const QString &name);
    void setSubject(const QString &subject);
    void setText(const QString &text);
    QString subject() const;
    QString text() const;
    QLazySharedPointer<Thread> thread() const;
private:
    friend class odb::access;
};

PRAGMA_DB(view object(Post))
struct OLOLORD_EXPORT PostCount
{
    PRAGMA_DB(column("count(" + Post::id_ + ")"))
    int count;
};

#endif // THREAD_H
