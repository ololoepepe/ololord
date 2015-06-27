#ifndef CONTENT_EDITAUDIOTAGS_H
#define CONTENT_EDITAUDIOTAGS_H

#include "controller/base.h"

#include "../global.h"

#include <cppcms/view.h>

#include <string>

namespace Content
{

struct OLOLORD_EXPORT EditAudioTags : public Base
{
    std::string audioTagAlbum;
    std::string audioTagAlbumText;
    std::string audioTagArtist;
    std::string audioTagArtistText;
    std::string audioTagTitle;
    std::string audioTagTitleText;
    std::string audioTagYear;
    std::string audioTagYearText;
    std::string currentBoardName;
    std::string fileName;
    unsigned long long postNumber;
};

}

#endif // CONTENT_EDITAUDIOTAGS_H
