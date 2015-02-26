#ifndef ECHOBOARD_H
#define ECHOBOARD_H

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

class OLOLORD_EXPORT echoBoard : public AbstractBoard
{
public:
    explicit echoBoard();
public:
    QString name() const;
    QString title(const QLocale &l) const;
protected:
    void beforeRenderBoard(const cppcms::http::request &req, Content::Board *c);
    void beforeRenderThread(const cppcms::http::request &req, Content::Thread *c);
    Content::Thread *createThreadController(const cppcms::http::request &req, QString &viewName);
    bool testParam(ParamType t, const QString &param, bool post, const QLocale &l, QString *error = 0) const;
};

#endif // ECHOBOARD_H
