#include "vgboard.h"

#include "translator.h"

#include <QLocale>
#include <QString>

#include <string>

vgBoard::vgBoard()
{
    //
}

QString vgBoard::name() const
{
    return "vg";
}

std::string vgBoard::title(const QLocale &l) const
{
    TranslatorStd ts(l);
    return ts.translate("vgBoard", "Video games", "board title");
}
