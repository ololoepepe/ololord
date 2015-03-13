#include "thread.h"

#include <BeQt>

#include <QByteArray>
#include <QDateTime>
#include <QDebug>
#include <QList>
#include <QSharedPointer>
#include <QString>
#include <QStringList>

#include <odb/qt/lazy-ptr.hxx>

Thread::Thread(const QString &board, quint64 number, const QDateTime &dateTime)
{
    id_ = 0L;
    board_ = board;
    number_ = number;
    dateTime_ = dateTime.toUTC();
    archived_ = false;
    fixed_ = false;
    postingEnabled_ = true;
}

Thread::Thread()
{
    //
}

quint64 Thread::id() const
{
    return id_;
}

QString Thread::board() const
{
    return board_;
}

quint64 Thread::number() const
{
    return number_;
}

QDateTime Thread::dateTime() const
{
    return QDateTime(dateTime_.date(), dateTime_.time(), Qt::UTC);
}

bool Thread::archived() const
{
    return archived_;
}

bool Thread::fixed() const
{
    return fixed_;
}

bool Thread::postingEnabled() const
{
    return postingEnabled_;
}

const Thread::Posts &Thread::posts() const
{
    return posts_;
}

Thread::Posts &Thread::posts()
{
    return posts_;
}

void Thread::setArchived(bool archived)
{
    archived_ = archived;
}

void Thread::setBoard(const QString &board)
{
    board_ = board;
}

void Thread::setDateTime(const QDateTime &dateTime)
{
    dateTime_ = dateTime;
}

void Thread::setFixed(bool fixed)
{
    fixed_ = fixed;
}

void Thread::setPostingEnabled(bool enabled)
{
    postingEnabled_ = enabled;
}

Post::Post()
{
    //
}

Post::Post(const QString &board, quint64 number, const QDateTime &dateTime, QSharedPointer<Thread> thread,
           const QString &posterIp, const QByteArray &password, const QByteArray &hashpass)
{
    id_ = 0L;
    board_ = board;
    number_ = number;
    dateTime_ = dateTime.toUTC();
    hashpass_ = hashpass;
    bannedFor_ = false;
    showTripcode_ = false;
    thread_ = thread;
    posterIp_ = posterIp;
    password_ = password;
}

quint64 Post::id() const
{
    return id_;
}

QString Post::board() const
{
    return board_;
}

quint64 Post::number() const
{
    return number_;
}

QDateTime Post::dateTime() const
{
    return QDateTime(dateTime_.date(), dateTime_.time(), Qt::UTC);
}

bool Post::bannedFor() const
{
    return bannedFor_;
}

bool Post::showTripcode() const
{
    return showTripcode_;
}

QString Post::email() const
{
    return email_;
}

QStringList Post::files() const
{
    return BeQt::deserialize(files_).toStringList();
}

QByteArray Post::hashpass() const
{
    return hashpass_;
}

QString Post::name() const
{
    return name_;
}

QByteArray Post::password() const
{
    return password_;
}

QString Post::posterIp() const
{
    return posterIp_;
}

void Post::setBannedFor(bool banned)
{
    bannedFor_ = banned;
}

void Post::setShowTripcode(bool show)
{
    showTripcode_ = show;
}

void Post::setEmail(const QString &email)
{
    email_ = email;
}

void Post::setFiles(const QStringList &files)
{
    files_ = BeQt::serialize(files);
}

void Post::setName(const QString &name)
{
    name_ = name;
}

void Post::setSubject(const QString &subject)
{
    subject_ = subject;
}

void Post::setText(const QString &text)
{
    text_ = text;
}

QString Post::subject() const
{
    return subject_;
}

QString Post::text() const
{
    return text_;
}

QLazySharedPointer<Thread> Post::thread() const
{
    return thread_;
}
