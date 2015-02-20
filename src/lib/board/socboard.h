#ifndef SOCBOARD_H
#define SOCBOARD_H

class QLocale;
class QString;

#include "abstractboard.h"
#include "../global.h"

class OLOLORD_EXPORT socBoard : public AbstractBoard
{
public:
    explicit socBoard();
public:
    QString defaultUserName(const QLocale &l) const;
    bool isHidden() const;
    QString name() const;
    QString title(const QLocale &l) const;
};

#endif // SOCBOARD_H
