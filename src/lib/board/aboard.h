#ifndef ABOARD_H
#define ABOARD_H

class QLocale;
class QString;

#include "abstractboard.h"
#include "../global.h"

#include <string>

class OLOLORD_EXPORT aBoard : public AbstractBoard
{
public:
    explicit aBoard();
public:
    QString name() const;
    std::string title(const QLocale &l) const;
};

#endif // ABOARD_H
