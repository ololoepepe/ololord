#ifndef RPGBOARD_H
#define RPGBOARD_H

class QLocale;
class QString;

#include "abstractboard.h"
#include "../global.h"

class rpgBoard : public AbstractBoard
{
public:
    explicit rpgBoard();
public:
    QString name() const;
    QString title(const QLocale &l) const;
};

#endif // RPGBOARD_H
