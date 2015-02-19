#ifndef PRBOARD_H
#define PRBOARD_H

namespace Content
{

class Board;
class Thread;

}

class QLocale;
class QString;

#include "abstractboard.h"

#include "tools.h"

class OLOLORD_EXPORT prBoard : public AbstractBoard
{
public:
    explicit prBoard();
public:
    bool isCaptchaValid(const Tools::PostParameters &params, QString &error, const QLocale &l) const;
    QString name() const;
    bool processCode() const;
    QString title(const QLocale &l) const;
protected:
    void beforeRenderBoard(Content::Board *c, const QLocale &l);
    void beforeRenderThread(Content::Thread *c, const QLocale &l);
    Content::Board *createBoardController(QString &viewName, const QLocale &l);
    Content::Thread *createThreadController(QString &viewName, const QLocale &l);
};

#endif // PRBOARD_H
