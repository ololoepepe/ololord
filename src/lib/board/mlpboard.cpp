#include "mlpboard.h"

#include "controller/board_video.h"
#include "controller/controller.h"
#include "tools.h"
#include "translator.h"

#include <BCoreApplication>

#include <QDateTime>
#include <QDebug>
#include <QLocale>
#include <QSettings>
#include <QString>
#include <QVariant>

#include <limits>
#include <string>

mlpBoard::mlpBoard()
{
    //
}

void mlpBoard::handleBoard(cppcms::application &app, unsigned int /*page*/)
{
    qsrand((uint) QDateTime::currentMSecsSinceEpoch());
    Content::BoardVideo c;
    TranslatorQt tq(app.request());
    TranslatorStd ts(app.request());
    Controller::initBase(c, app.request(), title(tq.locale()));
    c.altVideoText = ts.translate("mlpBoard", "Friendship is magic", "altVideoText");
    c.videoFileName = Tools::toStd(QString("friendship_is_magic_%1.webm").arg((qrand() % 2) + 1));
    c.videoType = "video/webm";
    app.render("board_video", c);
}

QString mlpBoard::name() const
{
    return "mlp";
}

bool mlpBoard::postingEnabled() const
{
    return false;
}

QString mlpBoard::title(const QLocale &l) const
{
    TranslatorQt tq(l);
    return tq.translate("mlpBoard", "My Little Pony", "board title");
}
