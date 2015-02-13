#ifndef HBOARD_H
#define HBOARD_H

class QLocale;
class QString;

#include "abstractboard.h"
#include "../global.h"

#include <string>

class OLOLORD_EXPORT hBoard : public AbstractBoard
{
public:
    explicit hBoard();
public:
    QString name() const;
    std::string title(const QLocale &l) const;
};

#endif // HBOARD_H
