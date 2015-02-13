#ifndef PRBOARD_H
#define PRBOARD_H

class QLocale;
class QString;

#include "abstractboard.h"
#include "../global.h"

#include <string>

class OLOLORD_EXPORT prBoard : public AbstractBoard
{
public:
    explicit prBoard();
public:
    QString name() const;
    bool processCode() const;
    std::string title(const QLocale &l) const;
};

#endif // PRBOARD_H
