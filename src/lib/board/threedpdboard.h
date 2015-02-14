#ifndef THREEDPDBOARD_H
#define THREEDPDBOARD_H

class QLocale;
class QString;

#include "abstractboard.h"
#include "../global.h"

class OLOLORD_EXPORT threedpdBoard : public AbstractBoard
{
public:
    explicit threedpdBoard();
public:
    QString name() const;
    QString title(const QLocale &l) const;
};

#endif // THREEDPDBOARD_H
