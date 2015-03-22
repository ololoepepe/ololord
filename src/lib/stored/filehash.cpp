#include "filehash.h"

#include <QByteArray>
#include <QString>

FileHash::FileHash(const QString &path, const QByteArray &hash)
{
    hash_ = hash;
    path_ = path;
}

FileHash::FileHash()
{
    //
}

QByteArray FileHash::hash() const
{
    return hash_;
}

QString FileHash::path() const
{
    return path_;
}
