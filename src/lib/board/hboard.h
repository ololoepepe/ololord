#ifndef HBOARD_H
#define HBOARD_H

class QLocale;
class QString;

#include "abstractboard.h"
#include "../global.h"

class OLOLORD_EXPORT hBoard : public AbstractBoard
{
public:
    explicit hBoard();
public:
    QString name() const;
    QString title(const QLocale &l) const;
};

#endif // HBOARD_H
