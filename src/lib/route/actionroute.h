#ifndef ACTIONROUTE_H
#define ACTIONROUTE_H

class AbstractBoard;

namespace Translator
{

class Qt;

}

class QStringList;

namespace cppcms
{

class application;

namespace http
{

class cookie;

}

}

#include "abstractroute.h"

#include <QString>

#include <string>

class OLOLORD_EXPORT ActionRoute : public AbstractRoute
{
public:
    explicit ActionRoute(cppcms::application &app);
public:
    static QStringList availableActions();
public:
    void handle(std::string action);
    unsigned int handlerArgumentCount() const;
    std::string key() const;
    int priority() const;
    std::string regex() const;
    std::string url() const;
private:
    void redirect(const QString &path = "settings");
    void setCookie(const QString &name, const QString &sourceName);
    bool testBoard(AbstractBoard *board, const QString &action, const QString &logTarget, const Translator::Qt &tq);
};

#endif // ACTIONROUTE_H
