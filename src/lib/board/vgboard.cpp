#include "vgboard.h"

#include "translator.h"

#include <QLocale>
#include <QString>

vgBoard::vgBoard()
{
    //
}

QString vgBoard::defaultUserName(const QLocale &l) const
{
    return TranslatorQt(l).translate("vgBoard", "PC Nobleman", "defaultUserName");
}

QString vgBoard::name() const
{
    return "vg";
}

QString vgBoard::title(const QLocale &l) const
{
    TranslatorQt tq(l);
    return tq.translate("vgBoard", "Video games", "board title");
}
