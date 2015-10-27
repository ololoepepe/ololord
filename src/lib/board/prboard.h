#ifndef PRBOARD_H
#define PRBOARD_H

namespace Content
{

class Board;
class Thread;

}

class QLocale;
class QString;

namespace cppcms
{

namespace http
{

class request;

}

}

#include "abstractboard.h"

#include "tools.h"

class OLOLORD_EXPORT prBoard : public AbstractBoard
{
public:
    explicit prBoard();
public:
    MarkupElements markupElements() const;
    QString name() const;
    QString supportedCaptchaEngines() const;
    QString title(const QLocale &l) const;
protected:
    void beforeRenderBoard(const cppcms::http::request &req, Content::Board *c);
    void beforeRenderThread(const cppcms::http::request &req, Content::Thread *c);
    Content::Board *createBoardController(const cppcms::http::request &req, QString &viewName);
    Content::Thread *createThreadController(const cppcms::http::request &req, QString &viewName);
};

#endif // PRBOARD_H
