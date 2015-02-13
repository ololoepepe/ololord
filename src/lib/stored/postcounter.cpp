#include "postcounter.h"

#include <QString>

PostCounter::PostCounter(const QString &board)
{
    board_ = board;
    lastPostNumber_ = 0L;
}

PostCounter::PostCounter()
{
    //
}

QString PostCounter::board() const
{
    return board_;
}

quint64 PostCounter::lastPostNumber() const
{
    return lastPostNumber_;
}

quint64 PostCounter::incrementLastPostNumber()
{
    return ++lastPostNumber_;
}
