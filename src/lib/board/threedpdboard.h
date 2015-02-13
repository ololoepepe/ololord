#ifndef THREEDPDBOARD_H
#define THREEDPDBOARD_H

class QLocale;
class QString;

#include "abstractboard.h"
#include "../global.h"

#include <string>

class OLOLORD_EXPORT threedpdBoard : public AbstractBoard
{
public:
    explicit threedpdBoard();
public:
    QString name() const;
    std::string title(const QLocale &l) const;
};

#endif // THREEDPDBOARD_H
