#ifndef EDITAUDIOTAGSROUTE_H
#define EDITAUDIOTAGSROUTE_H

namespace cppcms
{

class application;

}

#include "abstractroute.h"

#include <string>

class OLOLORD_EXPORT EditAudioTagsRoute : public AbstractRoute
{
public:
    explicit EditAudioTagsRoute(cppcms::application &app);
public:
    void handle();
    unsigned int handlerArgumentCount() const;
    std::string key() const;
    int priority() const;
    std::string regex() const;
    std::string url() const;
};

#endif // EDITAUDIOTAGSROUTE_H
