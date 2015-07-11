#include "mlpboard.h"

#include "controller/mlpboard.h"
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

void mlpBoard::handleBoard(cppcms::application &app, unsigned int page)
{
    if (!Tools::cookieValue(app.request(), "no_joke_mlp").compare("true", Qt::CaseInsensitive))
        return AbstractBoard::handleBoard(app, page);
    QString logTarget = name() + "/" + QString::number(page);
    if (page > 0)
        return Tools::log(app, "board", "fail:not_found", logTarget);
    qsrand((uint) QDateTime::currentMSecsSinceEpoch());
    Content::mlpBoard c;
    TranslatorQt tq(app.request());
    TranslatorStd ts(app.request());
    Controller::initBase(c, app.request(), title(tq.locale()));
    c.altVideoText = ts.translate("mlpBoard", "Friendship is magic", "altVideoText");
    c.buttonText = ts.translate("mlpBoard", "Hands off my pony!!!!!11", "buttonText");
    c.noJokeButtonText = ts.translate("mlpBoard", "Enough jokes, please!", "noJokeButtonText");
    c.videoFileName = Tools::toStd(QString("friendship_is_magic_%1.webm").arg((qrand() % 3) + 1));
    c.videoFileName2 = "bombanoolow.webm";
    c.videoType = "video/webm";
    Tools::render(app, "mlp_board", c);
    Tools::log(app, "board", "success", logTarget);
}

QString mlpBoard::name() const
{
    return "mlp";
}

QString mlpBoard::title(const QLocale &l) const
{
    TranslatorQt tq(l);
    return tq.translate("mlpBoard", "My Little Pony", "board title");
}
