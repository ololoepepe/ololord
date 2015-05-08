#ifndef DBOARD_H
#define DBOARD_H

namespace Content
{

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

class OLOLORD_EXPORT dBoard : public AbstractBoard
{
public:
    explicit dBoard();
public:
    bool beforeStoringNewPost(const cppcms::http::request &req, Post *post, const Tools::PostParameters &params,
                              bool thread, QString *error = 0, QString *description = 0);
    QString name() const;
    QString title(const QLocale &l) const;
    Content::Post toController(const Post &post, const cppcms::http::request &req, bool *ok = 0,
                               QString *error = 0) const;
};

#endif // DBOARD_H
