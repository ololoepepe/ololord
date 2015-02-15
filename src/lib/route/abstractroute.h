#ifndef ABSTRACTROUTE_H
#define ABSTRACTROUTE_H

namespace cppcms
{

class application;

}

#include "../global.h"

#include <string>

class OLOLORD_EXPORT AbstractRoute
{
protected:
    cppcms::application &application;
public:
    explicit AbstractRoute(cppcms::application &app);
    virtual ~AbstractRoute();
public:
    virtual bool duplicateWithSlashAppended() const;
    virtual void handle();
    virtual void handle(std::string path);
    virtual void handle(std::string path1, std::string path2);
    virtual void handle(std::string path1, std::string path2, std::string path3);
    virtual void handle(std::string path1, std::string path2, std::string path3, std::string path4);
    virtual unsigned int handlerArgumentCount() const = 0;
    virtual std::string key() const = 0;
    virtual int priority() const = 0;
    virtual std::string regex() const = 0;
    virtual std::string url() const = 0;
};

#endif // ABSTRACTROUTE_H
