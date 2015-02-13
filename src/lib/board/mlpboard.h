#ifndef MLPBOARD_H
#define MLPBOARD_H

class QLocale;
class QString;

namespace cppcms
{

class application;

}

#include "abstractboard.h"
#include "../global.h"

#include <string>

class OLOLORD_EXPORT mlpBoard : public AbstractBoard
{
public:
    explicit mlpBoard();
public:
    void handleBoard(cppcms::application &app, unsigned int page = 0);
    QString name() const;
    bool postingEnabled() const;
    std::string title(const QLocale &l) const;
};

#endif // MLPBOARD_H
