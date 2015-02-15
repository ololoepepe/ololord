#ifndef BOARDROUTE_H
#define BOARDROUTE_H

namespace cppcms
{

class application;

}

#include "abstractroute.h"

#include <string>

class OLOLORD_EXPORT BoardRoute : public AbstractRoute
{
public:
    enum Mode
    {
        BoardMode = 1,
        BoardPageMode,
        BoardRulesRoute
    };
private:
    const Mode mode;
public:
    explicit BoardRoute(cppcms::application &app, Mode m);
public:
    bool duplicateWithSlashAppended() const;
    void handle(std::string boardName);
    void handle(std::string boardName, std::string page);
    unsigned int handlerArgumentCount() const;
    std::string key() const;
    int priority() const;
    std::string regex() const;
    std::string url() const;
};

#endif // BOARDROUTE_H
