#include "hboard.h"

#include "translator.h"

#include <QLocale>
#include <QString>

#include <string>

hBoard::hBoard()
{
    //
}

QString hBoard::name() const
{
    return "h";
}

std::string hBoard::title(const QLocale &l) const
{
    TranslatorStd ts(l);
    return ts.translate("hBoard", "/h/entai", "board title");
}
