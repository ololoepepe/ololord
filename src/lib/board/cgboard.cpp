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

void cgBoard::handleBoard(cppcms::application &app, unsigned int /*page*/)
{
    Content::BoardImage c;
    TranslatorStd ts(app.request());
    Controller::initWithBase(&c, ts.locale());
    c.imageFileName = "drakeface.jpg";
    c.imageTitle = ts.translate("cgBoard", "No games", "imageTitle");
    c.pageTitle = title(ts.locale());
    app.render("board_image", c);
}

QString cgBoard::name() const
{
    return "cg";
}

bool cgBoard::postingEnabled() const
{
    return false;
}

std::string cgBoard::title(const QLocale &l) const
{
    TranslatorStd ts(l);
    return ts.translate("cgBoard", "Console games", "board title");
}
