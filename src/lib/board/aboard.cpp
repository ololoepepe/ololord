#include "aboard.h"

#include "translator.h"

#include <QLocale>
#include <QString>

aBoard::aBoard()
{
    //
}

QString aBoard::name() const
{
    return "a";
}

QString aBoard::title(const QLocale &l) const
{
    TranslatorQt tq(l);
    return tq.translate("aBoard", "/a/nime", "board title");
}
