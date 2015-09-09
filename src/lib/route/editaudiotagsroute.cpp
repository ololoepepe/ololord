#include "editaudiotagsroute.h"

#include "controller.h"
#include "controller/editaudiotags.h"
#include "database.h"
#include "tools.h"
#include "translator.h"

#include <BeQtGlobal>

#include <QDebug>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QVariantMap>

#include <cppcms/application.h>
#include <cppcms/http_request.h>

EditAudioTagsRoute::EditAudioTagsRoute(cppcms::application &app) :
    AbstractRoute(app)
{
    //
}

void EditAudioTagsRoute::handle()
{
    DDOS_A(100)
    Tools::GetParameters params = Tools::getParameters(application.request());
    QString boardName = params.value("board");
    quint64 postNumber = params.value("post").toULongLong();
    QString fileName = params.value("fileName");
    QString logTarget = boardName + "/" + QString::number(postNumber) + "/" + fileName;
    Tools::log(application, "edit_audio_tags", "begin", logTarget);
    QString err;
    if (!Controller::testRequestNonAjax(application, Controller::GetRequest, &err))
        return Tools::log(application, "edit_audio_tags", "fail:" + err, logTarget);
    TranslatorQt tq(application.request());
    if (Tools::hashpassString(application.request()).isEmpty()) {
        QString err = tq.translate("EditAudioTagsRoute", "Access error", "error");
        Controller::renderErrorNonAjax(application, err,
                                       tq.translate("EditAudioTagsRoute", "Not enough rights", "description"));
        Tools::log(application, "edit_audio_tags", "fail:" + err, logTarget);
        return;
    }
    if (boardName.isEmpty()) {
        QString err = tq.translate("EditAudioTagsRoute", "Invalid board name", "error");
        Controller::renderErrorNonAjax(application, err,
                                       tq.translate("EditAudioTagsRoute", "Board name is empty", "description"));
        Tools::log(application, "edit_audio_tags", "fail:" + err, logTarget);
        return;
    }
    if (!postNumber) {
        QString err = tq.translate("EditAudioTagsRoute", "Invalid post number", "error");
        Controller::renderErrorNonAjax(application, err,
                                       tq.translate("EditAudioTagsRoute", "Post number is null", "description"));
        Tools::log(application, "edit_audio_tags", "fail:" + err, logTarget);
        return;
    }
    if (fileName.isEmpty()) {
        QString err = tq.translate("EditAudioTagsRoute", "Invalid file name", "error");
        Controller::renderErrorNonAjax(application, err,
                                       tq.translate("EditAudioTagsRoute", "File name is empty", "description"));
        Tools::log(application, "edit_audio_tags", "fail:" + err, logTarget);
        return;
    }
    AbstractBoard::LockingWrapper board = AbstractBoard::board(boardName);
    if (board.isNull()) {
        QString err = tq.translate("EditAudioTagsRoute", "Unknown board", "error");
        Controller::renderErrorNonAjax(application, err,
                                       tq.translate("EditAudioTagsRoute", "There is no such board", "description"));
        Tools::log(application, "edit_audio_tags", "fail:" + err, logTarget);
        return;
    }
    bool ok = false;
    QVariantMap m = Database::getFileMetaData(fileName, &ok, &err, tq.locale()).toMap();
    if (!ok) {
        Controller::renderErrorNonAjax(application, tq.translate("EditAudioTagsRoute", "Internal error", "error"),
                                       err);
        Tools::log(application, "edit_audio_tags", "fail:" + err, logTarget);
        return;
    }
    TranslatorStd ts(tq);
    Content::EditAudioTags c;
    Controller::initBase(c, application.request(),
                         tq.translate("EditAudioTagsRoute", "Edit audio file tags", "pageTitle"));
    c.currentBoardName = Tools::toStd(boardName);
    c.postNumber = postNumber;
    c.fileName = Tools::toStd(fileName);
    c.audioTagAlbum = Tools::toStd(m.value("album").toString());
    c.audioTagAlbumText = ts.translate("EditAudioTagsRoute", "Album:", "audioTagAlbumText");
    c.audioTagArtist = Tools::toStd(m.value("artist").toString());
    c.audioTagArtistText = ts.translate("EditAudioTagsRoute", "Artist:", "audioTagArtistText");
    c.audioTagTitle = Tools::toStd(m.value("title").toString());
    c.audioTagTitleText = ts.translate("EditAudioTagsRoute", "Title:", "audioTagTitleText");
    c.audioTagYear = Tools::toStd(m.value("year").toString());
    c.audioTagYearText = ts.translate("EditAudioTagsRoute", "Year:", "audioTagYearText");
    Tools::render(application, "edit_audio_tags", c);
    Tools::log(application, "edit_audio_tags", "success", logTarget);
}

unsigned int EditAudioTagsRoute::handlerArgumentCount() const
{
    return 0;
}

std::string EditAudioTagsRoute::key() const
{
    return "edit_audio_tags";
}

int EditAudioTagsRoute::priority() const
{
    return 0;
}

std::string EditAudioTagsRoute::regex() const
{
    return "/edit_audio_tags";
}

std::string EditAudioTagsRoute::url() const
{
    return "/edit_audio_tags";
}
