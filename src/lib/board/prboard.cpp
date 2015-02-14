#include "prboard.h"

#include "tools.h"
#include "translator.h"

#include <QLocale>
#include <QString>

prBoard::prBoard()
{
    //
}

QString prBoard::name() const
{
    return "pr";
}

bool prBoard::processCode() const
{
    return true;
}

QString prBoard::title(const QLocale &l) const
{
    TranslatorQt tq(l);
    return tq.translate("prBoard", "/pr/ogramming", "board title");
}
