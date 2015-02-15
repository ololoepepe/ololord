#include "abstractroute.h"

#include <cppcms/application.h>

#include <string>

AbstractRoute::AbstractRoute(cppcms::application &app) :
    application(app)
{
    //
}

AbstractRoute::~AbstractRoute()
{
    //
}

bool AbstractRoute::duplicateWithSlashAppended() const
{
    return false;
}

void AbstractRoute::handle()
{
    //
}

void AbstractRoute::handle(std::string)
{
    //
}

void AbstractRoute::handle(std::string, std::string)
{
    //
}

void AbstractRoute::handle(std::string, std::string, std::string)
{
    //
}

void AbstractRoute::handle(std::string, std::string, std::string, std::string)
{
    //
}
