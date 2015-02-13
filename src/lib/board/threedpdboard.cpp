#include "threedpdboard.h"

#include "translator.h"

#include <QLocale>
#include <QString>

#include <string>

threedpdBoard::threedpdBoard()
{
    //
}

QString threedpdBoard::name() const
{
    return "3dpd";
}

std::string threedpdBoard::title(const QLocale &l) const
{
    TranslatorStd ts(l);
    return ts.translate("threedpdBoard", "3D pron", "board title");
}
