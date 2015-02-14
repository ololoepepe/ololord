#include "threedpdboard.h"

#include "translator.h"

#include <QLocale>
#include <QString>

threedpdBoard::threedpdBoard()
{
    //
}

QString threedpdBoard::name() const
{
    return "3dpd";
}

QString threedpdBoard::title(const QLocale &l) const
{
    TranslatorQt tq(l);
    return tq.translate("threedpdBoard", "3D pron", "board title");
}
