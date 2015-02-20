#include "rfboard.h"

#include "translator.h"

#include <QLocale>
#include <QString>

rfBoard::rfBoard()
{
    //
}

QString rfBoard::defaultUserName(const QLocale &l) const
{
    return TranslatorQt(l).translate("rfBoard", "Whiner", "defaultUserName");
}

QString rfBoard::name() const
{
    return "rf";
}

QString rfBoard::title(const QLocale &l) const
{
    TranslatorQt tq(l);
    return tq.translate("rfBoard", "Refuge", "board title");
}
