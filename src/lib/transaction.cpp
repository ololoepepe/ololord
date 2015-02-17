#include "transaction.h"

#include "tools.h"

#include <BDirTools>

#include <QDebug>
#include <QMap>
#include <QMutex>
#include <QMutexLocker>
#include <QString>

#include <odb/database.hxx>
#include <odb/exception.hxx>
#include <odb/schema-catalog.hxx>
#include <odb/sqlite/database.hxx>
#include <odb/transaction.hxx>

#include <exception>

static QMutex mutex;
static QMap<odb::transaction *, int> transactions;

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
    try {
        odb::transaction &t = odb::transaction::current();
        QMutexLocker locker(&mutex);
        if (transactions.value(&t) > 1) {
            transactions[&t] -= 1;
        } else {
            transactions.remove(&t);
            locker.unlock();
            try {
                t.commit();
                delete &t;
            } catch (const std::exception &e) {
                qDebug() << e.what();
                locker.relock();
                transactions.insert(&t, 1);
                return;
            }
        }
    } catch (const odb::not_in_transaction &e) {
        qDebug() << e.what();
        return;
    }
    finalized = true;
}

odb::database *Transaction::db() const
{
    try {
        odb::transaction &t = odb::transaction::current();
        return &t.database();
    } catch (const odb::not_in_transaction &e) {
        qDebug() << e.what();
        return 0;
    }
}

void Transaction::reset()
{
    if (!finalized)
        return;
    if (odb::transaction::has_current()) {
        QMutexLocker locker(&mutex);
        transactions[&odb::transaction::current()] += 1;
    } else {
        QString storagePath = Tools::storagePath();
        if (storagePath.isEmpty())
            return;
        QString fileName = storagePath + "/db.sqlite";
        if (!BDirTools::touch(fileName))
            return;
        try {
            odb::database *db = new odb::sqlite::database(Tools::toStd(fileName),
                                                          SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE);
            odb::transaction *t = new odb::transaction(db->begin());
            QMutexLocker locker(&mutex);
            transactions.insert(t, 1);
        } catch (const std::exception &e) {
            qDebug() << e.what();
            return;
        }
    }
    finalized = false;
}

void Transaction::rollback()
{
    if (finalized)
        return;
    try {
        odb::transaction &t = odb::transaction::current();
        QMutexLocker locker(&mutex);
        if (transactions.value(&t) > 1) {
            transactions[&t] -= 1;
        } else {
            transactions.remove(&t);
            locker.unlock();
            try {
                t.rollback();
                delete &t;
            } catch (const std::exception &e) {
                qDebug() << e.what();
                locker.relock();
                transactions.insert(&t, 1);
                return;
            }
        }
    } catch (const odb::not_in_transaction &e) {
        qDebug() << e.what();
        return;
    }
    finalized = true;
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
