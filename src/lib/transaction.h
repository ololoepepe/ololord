#ifndef TRANSACTION_H
#define TRANSACTION_H

namespace odb
{

class database;

}

#include "global.h"

class OLOLORD_EXPORT Transaction
{
public:
    const bool CommitOnDestruction;
private:
    bool finalized;
public:
    explicit Transaction(bool commitOnDestruction = false);
    ~Transaction();
public:
    void commit();
    odb::database *db() const;
    void reset();
    void rollback();
public:
    odb::database *operator ->() const;
    operator odb::database *() const;
    operator bool() const;
private:
    Q_DISABLE_COPY(Transaction)
};

#endif // TRANSACTION_H
