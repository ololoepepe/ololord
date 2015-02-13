#ifndef VGBOARD_H
#define VGBOARD_H

class QLocale;
class QString;

#include "abstractboard.h"
#include "../global.h"

#include <string>

class OLOLORD_EXPORT vgBoard : public AbstractBoard
{
public:
    explicit vgBoard();
public:
    QString name() const;
    std::string title(const QLocale &l) const;
};

#endif // VGBOARD_H
