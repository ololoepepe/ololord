#include "rpgboard.h"

#include "translator.h"

#include <QLocale>
#include <QString>

rpgBoard::rpgBoard()
{
    //
}

QString rpgBoard::name() const
{
    return "rpg";
}

QString rpgBoard::title(const QLocale &l) const
{
    return TranslatorQt(l).translate("rpgBoard", "Role-playing games", "board title");
}
