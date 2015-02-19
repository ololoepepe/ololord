#include "prboard.h"

#include "controller/board.h"
#include "controller/prboard.h"
#include "controller/prthread.h"
#include "controller/thread.h"
#include "tools.h"
#include "translator.h"

#include <QLocale>
#include <QString>

prBoard::prBoard()
{
    //
}

bool prBoard::isCaptchaValid(const Tools::PostParameters &params, QString &error, const QLocale &l) const
{
    return AbstractBoard::isCaptchaValid(params, error, l); //TODO
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

void prBoard::beforeRenderBoard(Content::Board *c, const QLocale &l)
{
    //TODO
}

void prBoard::beforeRenderThread(Content::Thread *c, const QLocale &l)
{
    //TODO
}

Content::Board *prBoard::createBoardController(QString &viewName, const QLocale &l)
{
    return AbstractBoard::createBoardController(viewName, l); //TODO
}

Content::Thread *prBoard::createThreadController(QString &viewName, const QLocale &l)
{
    return AbstractBoard::createThreadController(viewName, l); //TODO
}
