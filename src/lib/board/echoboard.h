#ifndef ECHOBOARD_H
#define ECHOBOARD_H

namespace Content
{

class Board;
class Thread;
class Post;

}

class Post;
class Thread;

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

#include <cppcms/json.h>

class OLOLORD_EXPORT echoBoard : public AbstractBoard
{
public:
    explicit echoBoard();
public:
    void beforeStoring(Post *post, const Tools::PostParameters &params, bool thread);
    bool editPost(const cppcms::http::request &req, cppcms::json::value &userData, Post &p, Thread &thread,
                  QString *error = 0);
    QString name() const;
    bool testParams(const Tools::PostParameters &params, bool post, const QLocale &l, QString *error) const;
    QString title(const QLocale &l) const;
    Content::Post toController(const Post &post, const cppcms::http::request &req, bool *ok = 0,
                               QString *error = 0) const;
    cppcms::json::object toJson(const Content::Post &post) const;
protected:
    void beforeRenderBoard(const cppcms::http::request &req, Content::Board *c);
    void beforeRenderThread(const cppcms::http::request &req, Content::Thread *c);
    Content::Board *createBoardController(const cppcms::http::request &req, QString &viewName);
    Content::Thread *createThreadController(const cppcms::http::request &req, QString &viewName);
};

#endif // ECHOBOARD_H
