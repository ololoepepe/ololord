#include "transaction.h"

#include "tools.h"

#include <BDirTools>

#include <QDebug>
#include <QMap>
#include <QMutex>
#include <QMutexLocker>
#include <QScopedPointer>
#include <QString>

#include <odb/database.hxx>
#include <odb/exception.hxx>
#include <odb/schema-catalog.hxx>
#include <odb/sqlite/connection.hxx>
#include <odb/sqlite/database.hxx>
#include <odb/transaction.hxx>

#include <exception>

class Hack : public odb::transaction
{
public:
    int counter;
private:
    static QMutex mutex;
public:
    explicit Hack(odb::transaction_impl *impl) :
        odb::transaction(impl)
    {
        mutex.lock();
        counter = 1;
    }
    ~Hack()
    {
        mutex.unlock();
    }
};

QMutex Hack::mutex;

Transaction::Transaction(bool commitOnDestruction) :
    CommitOnDestruction(commitOnDestruction)
{
    finalized = true;
    reset();
}

Transaction::~Transaction()
{
    if (CommitOnDestruction)
        commit();
    else
        rollback();
}

void Transaction::commit()
{
    if (finalized)
        return;
    finalized = true;
    try {
        Hack *h = reinterpret_cast<Hack *>(&odb::transaction::current());
        if (h->counter > 1) {
            h->counter -= 1;
        } else {
            odb::database *db = &h->database();
            try {
                h->commit();
                delete h;
                delete db;
            } catch (const odb::timeout &e) {
                Tools::log("Transaction::commit", e);
                delete h;
                delete db;
                throw e;
            }
        }
    } catch (const odb::not_in_transaction &e) {
        Tools::log("Transaction::commit", e);
        throw e;
    }
}

odb::database *Transaction::db() const
{
    try {
        odb::transaction &t = odb::transaction::current();
        return &t.database();
    } catch (const odb::not_in_transaction &e) {
        Tools::log("Transaction::db", e);
        return 0;
    }
}

void Transaction::reset()
{
    if (!finalized)
        return;
    if (odb::transaction::has_current()) {
        reinterpret_cast<Hack *>(&odb::transaction::current())->counter += 1;
    } else {
        QString storagePath = Tools::storagePath();
        if (storagePath.isEmpty())
            return;
        QString fileName = storagePath + "/db.sqlite";
        if (!BDirTools::touch(fileName))
            return;
        try {
            odb::sqlite::database *db = new odb::sqlite::database(Tools::toStd(fileName),
                                                                  SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE);
            new Hack(db->begin());
        } catch (const odb::exception &e) {
            Tools::log("Transaction::reset", e);
            return;
        }
    }
    finalized = false;
}

void Transaction::rollback()
{
    if (finalized)
        return;
    finalized = true;
    try {
        Hack *h = reinterpret_cast<Hack *>(&odb::transaction::current());
        if (h->counter > 1) {
            h->counter -= 1;
        } else {
            odb::database *db = &h->database();
            try {
                h->rollback();
                delete h;
                delete db;
            } catch (const odb::timeout &e) {
                Tools::log("Transaction::rollback", e);
                delete h;
                delete db;
                throw e;
            }
        }
    } catch (const odb::not_in_transaction &e) {
        Tools::log("Transaction::rollback", e);
        throw e;
    }
}

odb::database *Transaction::operator ->() const
{
    return db();
}

Transaction::operator odb::database *() const
{
    return db();
}

Transaction::operator bool() const
{
    return db();
}
