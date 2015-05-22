#ifndef CONTENT_PLAYLIST_H
#define CONTENT_PLAYLIST_H

#include "../global.h"
#include "base.h"

#include <cppcms/view.h>

#include <list>
#include <string>

namespace Content
{

struct OLOLORD_EXPORT Playlist : public Base
{
    std::string downloadPlaylistFileText;
    std::string removeFromPlaylistText;
    std::string unknownAlbumText;
    std::string unknownArtistText;
    std::string unknownTitleText;
};

}

#endif // CONTENT_PLAYLIST_H
