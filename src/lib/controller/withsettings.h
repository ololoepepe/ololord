#ifndef WITHSETTINGS_H
#define WITHSETTINGS_H

#include "../global.h"

#include <list>
#include <string>

struct OLOLORD_EXPORT WithSettings
{
    struct Locale
    {
        std::string country;
        std::string language;
        std::string name;
    public:
        bool operator ==(const Locale &other) const
        {
            return name == other.name;
        }
    };
public:
    Locale currentLocale;
    std::string localeLabelText;
    std::list<Locale> locales;
};

#endif // WITHSETTINGS_H
