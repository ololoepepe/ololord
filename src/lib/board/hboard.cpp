#include "hboard.h"

#include "translator.h"

#include <QLocale>
#include <QString>

hBoard::hBoard()
{
    //
}

QString hBoard::name() const
{
    return "h";
}

QString hBoard::title(const QLocale &l) const
{
    TranslatorQt tq(l);
    return tq.translate("hBoard", "/h/entai", "board title");
}
