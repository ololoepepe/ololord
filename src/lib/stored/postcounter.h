#ifndef POSTCOUNTER_H
#define POSTCOUNTER_H

#include "../global.h"

#include <QString>

#include <odb/core.hxx>

PRAGMA_DB(object table("postCounters"))
class OLOLORD_EXPORT PostCounter
{
private:
    PRAGMA_DB(id)
    QString board_;
    quint64 lastPostNumber_;
public:
    explicit PostCounter(const QString &board);
private:
    explicit PostCounter();
public:
    QString board() const;
    quint64 lastPostNumber() const;
    quint64 incrementLastPostNumber();
private:
    friend class odb::access;
};

#endif // POSTCOUNTER_H
