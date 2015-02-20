#ifndef VGBOARD_H
#define VGBOARD_H

class QLocale;
class QString;

#include "abstractboard.h"
#include "../global.h"

class OLOLORD_EXPORT vgBoard : public AbstractBoard
{
public:
    explicit vgBoard();
public:
    QString defaultUserName(const QLocale &l) const;
    QString name() const;
    QString title(const QLocale &l) const;
};

#endif // VGBOARD_H
