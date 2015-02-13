#ifndef RFBOARD_H
#define RFBOARD_H

class QLocale;
class QString;

#include "abstractboard.h"
#include "../global.h"

#include <string>

class OLOLORD_EXPORT rfBoard : public AbstractBoard
{
public:
    explicit rfBoard();
public:
    QString name() const;
    std::string title(const QLocale &l) const;
};

#endif // RFBOARD_H
