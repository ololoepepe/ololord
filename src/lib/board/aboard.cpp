#include "aboard.h"

#include "translator.h"

#include <QLocale>
#include <QString>

#include <string>

aBoard::aBoard()
{
    //
}

QString aBoard::name() const
{
    return "a";
}

std::string aBoard::title(const QLocale &l) const
{
    TranslatorStd ts(l);
    return ts.translate("aBoard", "/a/nime", "board title");
}
