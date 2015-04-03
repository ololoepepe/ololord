#include "transaction.h"

#include "tools.h"

#include <BDirTools>

#include <QDebug>
#include <QMap>
#include <QMutex>
#include <QMutexLocker>
#include <QRegExp>
#include <QString>

#include <odb/database.hxx>
#include <odb/exception.hxx>
#include <odb/schema-catalog.hxx>
#include <odb/sqlite/connection.hxx>
#include <odb/sqlite/database.hxx>
#include <odb/transaction.hxx>

#include <exception>

static void sqliteLike2(sqlite3_context *context, int argc, sqlite3_value **argv)
{
    if (argc != 2)
        return;
    const unsigned char *cwhere = sqlite3_value_text(argv[1]);
    const unsigned char *cwhat = sqlite3_value_text(argv[0]);
    QString where = QString::fromUtf8((char *) cwhere);
    QString what = QString::fromUtf8((char *) cwhat);
    QRegExp rx(what.replace('%', "*").replace('_', "?"), Qt::CaseInsensitive, QRegExp::Wildcard);
    sqlite3_result_int(context, where.contains(rx) ? 1 : 0);
}

static void sqliteLike3(sqlite3_context *context, int argc, sqlite3_value **argv)
{
    if (argc != 3)
        return;
    const unsigned char *cwhere = sqlite3_value_text(argv[1]);
    const unsigned char *cwhat = sqlite3_value_text(argv[0]);
    QString where = QString::fromUtf8((char *) cwhere);
    QString what = QString::fromUtf8((char *) cwhat);
    for (int i = what.size() - 1; i > 0; --i) {
        const QChar &c = what.at(i);
        if ('%' == c) {
            if ('\\' == what.at(i - 1)) {
                what.replace(i - 1, 2, "%");
                --i;
            } else {
                what.replace(i, 1, "*");
            }
        } else if ('_' == c) {
            if ('\\' == what.at(i - 1)) {
                what.replace(i - 1, 2, "_");
                --i;
            } else {
                what.replace(i, 1, "?");
            }
        }
    }
    QRegExp rx(what, Qt::CaseInsensitive, QRegExp::Wildcard);
    sqlite3_result_int(context, where.contains(rx) ? 1 : 0);
}

class Hack : public odb::transaction
{
public:
    int counter;
public:
    explicit Hack(odb::transaction_impl *impl) :
        odb::transaction(impl)
    {
        counter = 1;
    }
};

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
        Hack *h = reinterpret_cast<Hack *>(&odb::transaction::current());
        if (h->counter > 1) {
            h->counter -= 1;
        } else {
            try {
                h->commit();
                odb::database *db = &h->database();
                delete h;
                delete db;
            } catch (const std::exception &e) {
                qDebug() << e.what();
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
        reinterpret_cast<Hack *>(&odb::transaction::current())->counter += 1;
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
            odb::sqlite::connection *c = dynamic_cast<odb::sqlite::connection *>(db->connection().get());
            sqlite3_create_function(c->handle(), "like", 2, SQLITE_UTF8, 0, &sqliteLike2, 0, 0);
            sqlite3_create_function(c->handle(), "like", 3, SQLITE_UTF8, 0, &sqliteLike3, 0, 0);
            new Hack(db->begin());
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
        Hack *h = reinterpret_cast<Hack *>(&odb::transaction::current());
        if (h->counter > 1) {
            h->counter -= 1;
        } else {
            try {
                h->rollback();
                odb::database *db = &h->database();
                delete h;
                delete db;
            } catch (const std::exception &e) {
                qDebug() << e.what();
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
