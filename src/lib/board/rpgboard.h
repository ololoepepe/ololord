#ifndef RPGBOARD_H
#define RPGBOARD_H

namespace Content
{

class Board;
class Post;
class Thread;

}

class Post;

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

#include "../global.h"
#include "tools.h"

#include <cppcms/json.h>

class rpgBoard : public AbstractBoard
{
public:
    explicit rpgBoard();
public:
    bool beforeStoringEditedPost(const cppcms::http::request &req, cppcms::json::value &userData, Post &p,
                                 Thread &thread, QString *error = 0);
    bool beforeStoringNewPost(const cppcms::http::request &req, Post *post, const Tools::PostParameters &params,
                              bool thread, QString *error = 0, QString *description = 0);
    QString name() const;
    QString title(const QLocale &l) const;
    cppcms::json::object toJson(const Content::Post &post, const cppcms::http::request &req) const;
protected:
    void beforeRenderBoard(const cppcms::http::request &req, Content::Board *c);
    void beforeRenderThread(const cppcms::http::request &req, Content::Thread *c);
    Content::Board *createBoardController(const cppcms::http::request &req, QString &viewName);
    Content::Thread *createThreadController(const cppcms::http::request &req, QString &viewName);
};

#endif // RPGBOARD_H
