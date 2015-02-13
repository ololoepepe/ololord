#ifndef CGBOARD_H
#define CGBOARD_H

class QLocale;
class QString;

namespace cppcms
{

class application;

}

#include "abstractboard.h"
#include "../global.h"

#include <string>

class OLOLORD_EXPORT cgBoard : public AbstractBoard
{
public:
    explicit cgBoard();
public:
    void handleBoard(cppcms::application &app, unsigned int page = 0);
    QString name() const;
    bool postingEnabled() const;
    std::string title(const QLocale &l) const;
};

#endif // CGBOARD_H
