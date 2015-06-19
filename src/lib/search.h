#ifndef OLOLORD_SEARCH_H
#define OLOLORD_SEARCH_H

class QByteArray;
class QLocale;

#include "global.h"

#include <BCoreApplication>

#include <QHash>
#include <QMap>
#include <QSet>
#include <QString>
#include <QStringList>

namespace Search
{

typedef QSet<quint32> PositionSet;
typedef QMap<quint64, PositionSet> PostMap;
typedef QHash<QString, PostMap> BoardMap;

struct OLOLORD_EXPORT Query
{
    QStringList requiredPhrases;
    QStringList excludedPhrases;
    QStringList possiblePhrases;
public:
    bool isValid() const;
};

OLOLORD_EXPORT void addToIndex(const QString &boardName, quint64 postNumber, const QString &text);
OLOLORD_EXPORT void clearIndex();
OLOLORD_EXPORT BoardMap find(const Query &query, const QString &boardName, bool *ok = 0, QString *error = 0,
                             const QLocale &l = BCoreApplication::locale());
OLOLORD_EXPORT BoardMap find(const Query &query, bool *ok = 0, QString *error = 0,
                             const QLocale &l = BCoreApplication::locale());
OLOLORD_EXPORT bool isModified();
OLOLORD_EXPORT Query query(const QString &q, bool *ok = 0, QString *error = 0,
                           const QLocale &l = BCoreApplication::locale());
OLOLORD_EXPORT int rebuildIndex(QString *error = 0, const QLocale &l = BCoreApplication::locale());
OLOLORD_EXPORT void removeFromIndex(const QString &boardName, quint64 postNumber, const QString &text);
OLOLORD_EXPORT void restoreIndex(const QByteArray &data);
OLOLORD_EXPORT QByteArray saveIndex();

}

#endif // OLOLORD_SEARCH_H
