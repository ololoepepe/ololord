#include "search.h"

#include "database.h"
#include "translator.h"

#include <BeQt>
#include <BTextTools>

#include <QByteArray>
#include <QChar>
#include <QDataStream>
#include <QDebug>
#include <QHash>
#include <QIODevice>
#include <QLocale>
#include <QMultiMap>
#include <QReadLocker>
#include <QReadWriteLock>
#include <QString>
#include <QStringList>
#include <QWriteLocker>

namespace Search
{

bool Query::isValid() const
{
    return !requiredPhrases.isEmpty() || !possiblePhrases.isEmpty();
}

typedef QMap<QString, BoardMap> WordMap;

static WordMap index;
static QReadWriteLock indexLock(QReadWriteLock::Recursive);

static BoardMap complement(const BoardMap &boards1, const BoardMap &boards2)
{
    BoardMap boards;
    for (BoardMap::ConstIterator posts1 = boards1.begin(); posts1 != boards1.end(); ++posts1) {
        BoardMap::ConstIterator posts2 = boards2.find(posts1.key());
        if (posts2 != boards2.end()) {
            PostMap posts;
            for (PostMap::ConstIterator positions1 = posts1->begin(); positions1 != posts1->end(); ++positions1) {
                PostMap::ConstIterator positions2 = posts2->find(positions1.key());
                if (positions2 != posts2->end())
                    continue;
                posts.insert(positions1.key(), positions1.value());
            }
            if (posts.isEmpty())
                continue;
            boards.insert(posts1.key(), posts);
        } else {
            boards.insert(posts1.key(), posts1.value());
        }
    }
    return boards;
}

static BoardMap followedBy(const BoardMap &followed, const BoardMap &by)
{
    BoardMap boards;
    for (BoardMap::ConstIterator posts1 = followed.begin(); posts1 != followed.end(); ++posts1) {
        BoardMap::ConstIterator posts2 = by.find(posts1.key());
        if (posts2 == by.end())
            continue;
        PostMap posts;
        for (PostMap::ConstIterator positions1 = posts1->begin(); positions1 != posts1->end(); ++positions1) {
            PostMap::ConstIterator positions2 = posts2->find(positions1.key());
            if (positions2 == posts2->end())
                continue;
            PositionSet set;
            for (PositionSet::ConstIterator pos1 = positions1->begin(); pos1 != positions1->end(); ++pos1) {
                const quint32 &pos = *pos1;
                if (!positions2->contains(pos + 1))
                    continue;
                set.insert(pos + 1);
            }
            if (set.isEmpty())
                continue;
            posts.insert(positions1.key(), set);
        }
        if (posts.isEmpty())
            continue;
        boards.insert(posts1.key(), posts);
    }
    return boards;
}

static BoardMap intersection(const BoardMap &boards1, const BoardMap &boards2)
{
    BoardMap boards;
    for (BoardMap::ConstIterator posts1 = boards1.begin(); posts1 != boards1.end(); ++posts1) {
        BoardMap::ConstIterator posts2 = boards2.find(posts1.key());
        if (posts2 == boards2.end())
            continue;
        PostMap posts;
        for (PostMap::ConstIterator positions1 = posts1->begin(); positions1 != posts1->end(); ++positions1) {
            PostMap::ConstIterator positions2 = posts2->find(positions1.key());
            if (positions2 == posts2->end())
                continue;
            posts.insert(positions1.key(), positions1.value());
        }
        if (posts.isEmpty())
            continue;
        boards.insert(posts1.key(), posts);
    }
    return boards;
}

static BoardMap sum(const BoardMap &boards1, const BoardMap &boards2)
{
    BoardMap boards = boards1;
    for (BoardMap::ConstIterator posts2 = boards2.begin(); posts2 != boards2.end(); ++posts2) {
        BoardMap::Iterator posts = boards.find(posts2.key());
        if (posts != boards.end()) {
            for (PostMap::ConstIterator positions2 = posts2->begin(); positions2 != posts2->end(); ++positions2) {
                PostMap::Iterator positions = posts->find(positions2.key());
                if (positions != posts->end()) {
                    for (PositionSet::ConstIterator pos2 = positions2->begin(); pos2 != positions2->end(); ++pos2)
                        positions->insert(*pos2);
                } else {
                    posts->insert(positions2.key(), positions2.value());
                }
            }
        } else {
            boards.insert(posts2.key(), posts2.value());
        }
    }
    return boards;
}

static QStringList words(const QString &text)
{
    QStringList list;
    QString word;
    foreach (const QChar &c, text) {
        if (c.isLetterOrNumber()) {
            word += c;
        } else if (!word.isEmpty()) {
            list << word.toLower();
            word.clear();
        }
    }
    if (!word.isEmpty())
        list << word;
    return list;
}

static PostMap findHelper(BoardMap::ConstIterator boards)
{
    PostMap p;
    for (PostMap::ConstIterator posts = boards->begin(); posts != boards->end(); ++posts)
        p.insert(posts.key(), posts.value());
    return p;
}

static BoardMap findWord(const QString &w, const QString &boardName = QString())
{
    if (w.isEmpty())
        return BoardMap();
    WordMap::ConstIterator words = index.find(w);
    if (index.end() == words)
        return BoardMap();
    BoardMap m;
    if (!boardName.isEmpty()) {
        BoardMap::ConstIterator boards = words->find(boardName);
        if (boards == words->end())
            return BoardMap();
        m.insert(boardName, findHelper(boards));
    } else {
        for (BoardMap::ConstIterator boards = words->begin(); boards != words->end(); ++boards)
            m.insert(boards.key(), findHelper(boards));
    }
    return m;
}

static BoardMap findPhrase(const QString &phrase, const QString &boardName = QString())
{
    QStringList list = words(phrase);
    if (list.isEmpty())
        return BoardMap();
    if (list.size() == 1)
        return findWord(list.first(), boardName);
    BoardMap m = findWord(list.first(), boardName);
    foreach (int i, bRangeD(1, list.size() - 1)) {
        if (m.isEmpty())
            break;
        m = followedBy(m, findWord(list.at(i), boardName));
    }
    return m;
}

void addToIndex(const QString &boardName, quint64 postNumber, const QString &text)
{
    if (boardName.isEmpty() || !postNumber || text.isEmpty())
        return;
    QStringList list = words(text);
    if (list.isEmpty())
        return;
    QWriteLocker locker(&indexLock);
    foreach (int i, bRangeD(0, list.size() - 1))
        index[list.at(i)][boardName][postNumber].insert(quint32(i));
}

void clearIndex()
{
    QWriteLocker locker(&indexLock);
    index.clear();
}

BoardMap find(const Query &q, const QString &boardName, bool *ok, QString *error, const QLocale &l)
{
    TranslatorQt tq(l);
    if (!q.isValid())
        return bRet(ok, false, error, tq.translate("Search", "Invalid search query", "error"), BoardMap());
    QReadLocker locker(&indexLock);
    BoardMap m;
    if (!q.requiredPhrases.isEmpty())
        m = findPhrase(q.requiredPhrases.first(), boardName);
    foreach (int i, bRangeD(1, q.requiredPhrases.size() - 1))
        m = intersection(m, findPhrase(q.requiredPhrases.at(i), boardName));
    foreach (const QString &phrase, q.possiblePhrases)
        m = sum(m, findPhrase(phrase, boardName));
    foreach (const QString &phrase, q.excludedPhrases) {
        if (m.isEmpty())
            break;
        m = complement(m, findPhrase(phrase, boardName));
    }
    return bRet(ok, true, error, QString(), m);
}

BoardMap find(const Query &query, bool *ok, QString *error, const QLocale &l)
{
    return find(query, "", ok, error, l);
}

Query query(const QString &q, bool *ok, QString *error, const QLocale &l)
{
    bool b = false;
    TranslatorQt tq(l);
    QStringList phrases = BTextTools::splitCommand(q, &b);
    if (!b)
        return bRet(ok, false, error, tq.translate("Search", "Invalid search query", "error"), Query());
    Query qq;
    foreach (const QString &phrase, phrases) {
        if (phrase.startsWith('+'))
            qq.requiredPhrases << phrase.mid(1).toLower();
        else if (phrase.startsWith('-'))
            qq.excludedPhrases << phrase.mid(1).toLower();
        else
            qq.possiblePhrases << phrase.toLower();
    }
    return bRet(ok, true, error, QString(), qq);
}

void removeFromIndex(const QString &boardName, quint64 postNumber, const QString &text)
{
    if (boardName.isEmpty() || !postNumber || text.isEmpty())
        return;
    QStringList list = words(text);
    if (list.isEmpty())
        return;
    QWriteLocker locker(&indexLock);
    foreach (int i, bRangeD(0, list.size() - 1)) {
        const QString &word = list.at(i);
        BoardMap &boards = index[word];
        PostMap &posts = boards[boardName];
        PositionSet &positions = posts[postNumber];
        positions.remove(quint32(i));
        if (positions.isEmpty())
            posts.remove(postNumber);
        if (posts.isEmpty())
            boards.remove(boardName);
        if (boards.isEmpty())
            index.remove(word);
    }
}

bool reloadIndex(QString *error, const QLocale &l)
{
    QWriteLocker locker(&indexLock);
    index.clear();
    return Database::reloadPostIndex(error, l);
}

void restoreIndex(const QByteArray &data)
{
    QDataStream ds(data);
    ds.setVersion(BeQt::DataStreamVersion);
    QWriteLocker locker(&indexLock);
    ds >> index;
}

QByteArray saveIndex()
{
    QByteArray data;
    QDataStream out(&data, QIODevice::WriteOnly);
    QReadLocker locker(&indexLock);
    out << index;
    return data;
}

}
