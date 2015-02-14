#include "bboard.h"

#include "translator.h"

#include <QLocale>
#include <QString>

bBoard::bBoard()
{
    //
}

QString bBoard::name() const
{
    return "b";
}

QString bBoard::title(const QLocale &l) const
{
    TranslatorQt tq(l);
    return tq.translate("bBoard", "/b/rotherhood", "board title");
}
