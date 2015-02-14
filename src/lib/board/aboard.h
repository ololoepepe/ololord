#ifndef ABOARD_H
#define ABOARD_H

class QLocale;
class QString;

#include "abstractboard.h"
#include "../global.h"

class OLOLORD_EXPORT aBoard : public AbstractBoard
{
public:
    explicit aBoard();
public:
    QString name() const;
    QString title(const QLocale &l) const;
};

#endif // ABOARD_H
