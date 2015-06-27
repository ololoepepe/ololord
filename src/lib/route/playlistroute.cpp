#include "playlistroute.h"

#include "controller/controller.h"
#include "controller/playlist.h"
#include "tools.h"
#include "translator.h"

#include <BeQtGlobal>

#include <QDebug>
#include <QString>
#include <QStringList>

#include <cppcms/application.h>
#include <cppcms/http_request.h>

PlaylistRoute::PlaylistRoute(cppcms::application &app) :
    AbstractRoute(app)
{
    //
}

void PlaylistRoute::handle()
{
    Tools::log(application, "playlist", "begin");
    QString err;
    if (!Controller::testRequestNonAjax(application, Controller::GetRequest, &err))
        return Tools::log(application, "playlist", "fail:" + err);
    Content::Playlist c;
    TranslatorQt tq(application.request());
    TranslatorStd ts(tq.locale());
    Controller::initBase(c, application.request(), tq.translate("PlaylistRoute", "Playlist", "pageTitle"));
    c.downloadPlaylistFileText = ts.translate("PlaylistRoute", "Download file", "downloadPlaylistFileText");
    c.removeFromPlaylistText = ts.translate("PlaylistRoute", "Remove from playlist", "removeFromPlaylistText");
    c.unknownAlbumText = ts.translate("PlaylistRoute", "Unknown album", "unknownAlbumText");
    c.unknownArtistText = ts.translate("PlaylistRoute", "Unknown artist", "unknownArtistText");
    c.unknownTitleText = ts.translate("PlaylistRoute", "Unknown title", "unknownTitleText");
    Tools::render(application, "playlist", c);
    Tools::log(application, "playlist", "success");
}

unsigned int PlaylistRoute::handlerArgumentCount() const
{
    return 0;
}

std::string PlaylistRoute::key() const
{
    return "playlist";
}

int PlaylistRoute::priority() const
{
    return 0;
}

std::string PlaylistRoute::regex() const
{
    return "/playlist";
}

std::string PlaylistRoute::url() const
{
    return "/playlist";
}
