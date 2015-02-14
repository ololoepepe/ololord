#ifndef CONTENT_BASE_H
#define CONTENT_BASE_H

#include "../global.h"
#include "board/abstractboard.h"

#include <cppcms/view.h>

#include <list>
#include <string>

namespace Content
{

struct OLOLORD_EXPORT Base : public cppcms::base_content
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
    AbstractBoard::BoardInfoList boards;
    Locale currentLocale;
    std::string localeLabelText;
    std::list<Locale> locales;
    std::string pageTitle;
    std::string sitePathPrefix;
    std::string toHomePageText;
};

}

#endif // CONTENT_BASE_H
