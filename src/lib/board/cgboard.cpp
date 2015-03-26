#include "cgboard.h"

#include "controller/board_image.h"
#include "controller/controller.h"
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
    QString logTarget = name() + "/" + QString::number(page);
    if (page > 0)
        return Tools::log(app, "board", "fail:not_found", logTarget);
    Content::BoardImage c;
    TranslatorQt tq(app.request());
    TranslatorStd ts(app.request());
    Controller::initBase(c, app.request(), title(tq.locale()));
    c.imageFileName = "drakeface.jpg";
    c.imageTitle = ts.translate("cgBoard", "No games", "imageTitle");
    app.render("board_image", c);
    return Tools::log(app, "board", "success", logTarget);
}

QString cgBoard::name() const
{
    return "cg";
}

bool cgBoard::postingEnabled() const
{
    return false;
}

QString cgBoard::title(const QLocale &l) const
{
    TranslatorQt tq(l);
    return tq.translate("cgBoard", "Console games", "board title");
}
