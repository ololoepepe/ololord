#ifndef FILEHASH_H
#define FILEHASH_H

#include "../global.h"

#include <QByteArray>
#include <QString>

#include <odb/core.hxx>

PRAGMA_DB(object table("fileHashes"))
class OLOLORD_EXPORT FileHash
{
private:
    PRAGMA_DB(id)
    QString path_;
    PRAGMA_DB(not_null)
    QByteArray hash_;
public:
    explicit FileHash(const QString &path, const QByteArray &hash);
private:
    explicit FileHash();
public:
    QByteArray hash() const;
    QString path() const;
private:
    friend class odb::access;
};

PRAGMA_DB(view object(FileHash))
struct OLOLORD_EXPORT FileHashCount
{
    PRAGMA_DB(column("count(" + FileHash::path_ + ")"))
    int count;
};

#endif // FILEHASH_H
