#ifndef INTBOARD_H
#define INTBOARD_H

class QLocale;
class QString;

#include "abstractboard.h"
#include "../global.h"

class OLOLORD_EXPORT intBoard : public AbstractBoard
{
public:
    explicit intBoard();
public:
    QString name() const;
    QString title(const QLocale &l) const;
    bool showWhois() const;
};

#endif // INTBOARD_H
