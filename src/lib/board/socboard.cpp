#include "socboard.h"

#include "translator.h"

#include <QLocale>
#include <QString>

socBoard::socBoard()
{
    //
}

QString socBoard::defaultUserName(const QLocale &l) const
{
    return TranslatorQt(l).translate("socBoard", "Life of the party", "defaultUserName");
}

bool socBoard::isHidden() const
{
    return true;
}

QString socBoard::name() const
{
    return "soc";
}

QString socBoard::title(const QLocale &l) const
{
    TranslatorQt tq(l);
    return tq.translate("socBoard", "Communication", "board title");
}
