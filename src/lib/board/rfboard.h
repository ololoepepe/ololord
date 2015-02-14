#ifndef RFBOARD_H
#define RFBOARD_H

class QLocale;
class QString;

#include "abstractboard.h"
#include "../global.h"

class OLOLORD_EXPORT rfBoard : public AbstractBoard
{
public:
    explicit rfBoard();
public:
    QString name() const;
    QString title(const QLocale &l) const;
};

#endif // RFBOARD_H
