#include "bboard.h"

#include "translator.h"

#include <QLocale>
#include <QString>

#include <string>

bBoard::bBoard()
{
    //
}

QString bBoard::name() const
{
    return "b";
}

std::string bBoard::title(const QLocale &l) const
{
    TranslatorStd ts(l);
    return ts.translate("bBoard", "/b/rotherhood", "board title");
}
