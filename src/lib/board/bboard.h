#ifndef BBOARD_H
#define BBOARD_H

class QLocale;
class QString;

#include "abstractboard.h"
#include "../global.h"

#include <string>

class OLOLORD_EXPORT bBoard : public AbstractBoard
{
public:
    explicit bBoard();
public:
    QString name() const;
    std::string title(const QLocale &l) const;
};

#endif // BBOARD_H
