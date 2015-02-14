#ifndef PRBOARD_H
#define PRBOARD_H

class QLocale;
class QString;

#include "abstractboard.h"
#include "../global.h"

class OLOLORD_EXPORT prBoard : public AbstractBoard
{
public:
    explicit prBoard();
public:
    QString name() const;
    bool processCode() const;
    QString title(const QLocale &l) const;
};

#endif // PRBOARD_H
