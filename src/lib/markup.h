#ifndef MARKUP_H
#define MARKUP_H

namespace Database
{

class RefMap;

}

class QString;

#include "global.h"

namespace Markup
{

enum MarkupLanguage
{
    NoLanguage = 0x00,
    ExtendedWakabaMarkLanguage = 0x01,
    BBCodeLanguage = 0x02,
    AllLanguages = ExtendedWakabaMarkLanguage | BBCodeLanguage
};

OLOLORD_EXPORT QString processPostText(QString text, const QString &boardName, Database::RefMap *referencedPosts = 0,
                                       quint64 deletedPost = 0, MarkupLanguage languages = AllLanguages);
OLOLORD_EXPORT QString toHtml(const QString &s);
OLOLORD_EXPORT void toHtml(QString *s);

}

#endif // MARKUP_H
