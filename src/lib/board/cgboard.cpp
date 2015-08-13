#include "cgboard.h"

#include "controller.h"
#include "controller/cgboard.h"
#include "tools.h"
#include "translator.h"

#include <BCoreApplication>

#include <QLocale>
#include <QSettings>
#include <QString>
#include <QVariant>

#include <string>

cgBoard::cgBoard()
{
    //
}

void cgBoard::handleBoard(cppcms::application &app, unsigned int page)
{
    if (!Tools::cookieValue(app.request(), "no_joke_cg").compare("true", Qt::CaseInsensitive))
        return AbstractBoard::handleBoard(app, page);
    QString logTarget = name() + "/" + QString::number(page);
    if (page > 0)
        return Tools::log(app, "board", "fail:not_found", logTarget);
    Content::cgBoard c;
    TranslatorQt tq(app.request());
    TranslatorStd ts(app.request());
    Controller::initBase(c, app.request(), title(tq.locale()));
    c.imageFileName = "drakeface.jpg";
    c.imageTitle = ts.translate("cgBoard", "No games", "imageTitle");
    c.noJokeButtonText = ts.translate("cgBoard", "But there ARE games!", "imageTitle");
    Tools::render(app, "cg_board", c);
    return Tools::log(app, "board", "success", logTarget);
}

QString cgBoard::name() const
{
    return "cg";
}

QString cgBoard::title(const QLocale &l) const
{
    TranslatorQt tq(l);
    return tq.translate("cgBoard", "Console games", "board title");
}
