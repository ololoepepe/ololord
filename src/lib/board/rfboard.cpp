#include "rfboard.h"

#include "translator.h"

#include <QLocale>
#include <QString>

#include <string>

rfBoard::rfBoard()
{
    //
}

QString rfBoard::name() const
{
    return "rf";
}

std::string rfBoard::title(const QLocale &l) const
{
    TranslatorStd ts(l);
    return ts.translate("rfBoard", "Refuge", "board title");
}
