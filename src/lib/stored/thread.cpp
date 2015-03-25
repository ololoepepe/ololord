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
    draft_ = false;
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

bool Thread::draft() const
{
    return draft_;
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

void Thread::setDraft(bool draft)
{
    draft_ = draft;
}

Post::RefKey::RefKey()
{
    postNumber = 0;
}

Post::RefKey::RefKey(const QString &board, quint64 post)
{
    boardName = board;
    postNumber = post;
}

bool Post::RefKey::isValid() const
{
    return !boardName.isEmpty() && postNumber;
}

bool Post::RefKey::operator <(const RefKey &other) const
{
    return boardName < other.boardName || (boardName == other.boardName && postNumber < other.postNumber);
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
    rawHtml_ = false;
    draft_ = false;
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

bool Post::addReferencedBy(const RefKey &key, quint64 threadNumber)
{
    if (!key.isValid() || !threadNumber)
        return false;
    RefMap map = referencedBy();
    int sz = map.size();
    map.insert(key, threadNumber);
    if (map.size() == sz)
        return false;
    setReferencedBy(map);
    return true;
}

bool Post::addReferencedBy(const QString &boardName, quint64 postNumber, quint64 threadNumber)
{
    return addReferencedBy(RefKey(boardName, postNumber), threadNumber);
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

bool Post::draft() const
{
    return draft_;
}

QString Post::posterIp() const
{
    return posterIp_;
}

bool Post::rawHtml() const
{
    return rawHtml_;
}

QString Post::rawText() const
{
    return rawText_;
}

Post::RefMap Post::referencedBy() const
{
    RefMap map;
    foreach (const QVariant &v, BeQt::deserialize(referencedBy_).toList()) {
        QVariantMap m = v.toMap();
        RefKey key(m.value("boardName").toString(), m.value("postNumber").toULongLong());
        if (!key.isValid())
            continue;
        quint64 t = m.value("threadNumber").toULongLong();
        if (!t)
            continue;
        map.insert(key, t);
    }
    return map;
}

bool Post::removeReferencedBy(const RefKey &key)
{
    if (!key.isValid())
        return false;
    RefMap map = referencedBy();
    if (map.remove(key) < 1)
        return false;
    setReferencedBy(map);
    return true;
}

bool Post::removeReferencedBy(const QString &boardName, quint64 postNumber)
{
    return removeReferencedBy(RefKey(boardName, postNumber));
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

void Post::setDraft(bool draft)
{
    draft_ = draft;
}

void Post::setRawHtml(bool raw)
{
    rawHtml_ = raw;
}

void Post::setRawText(const QString &text)
{
    rawText_ = text;
}

void Post::setReferencedBy(const RefMap &map)
{
    QVariantList vl;
    foreach (const RefKey &key, map.keys()) {
        if (!key.isValid())
            continue;
        quint64 t = map.value(key);
        if (!t)
            continue;
        QVariantMap m;
        m.insert("boardName", key.boardName);
        m.insert("postNumber", key.postNumber);
        m.insert("threadNumber", t);
        vl << m;
    }
    referencedBy_ = BeQt::serialize(vl);
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
