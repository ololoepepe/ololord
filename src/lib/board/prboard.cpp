#include "prboard.h"

#include "tools.h"
#include "translator.h"

#include <QLocale>
#include <QString>

#include <string>

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

std::string prBoard::title(const QLocale &l) const
{
    TranslatorStd ts(l);
    return ts.translate("prBoard", "/pr/ogramming", "board title");
}
