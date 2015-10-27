#ifndef ACTIONROUTE_H
#define ACTIONROUTE_H

class AbstractBoard;

namespace Translator
{

class Qt;

}

class QStringList;

namespace cppcms
{

class application;

}

#include "abstractroute.h"
#include "tools.h"

#include <QMap>
#include <QString>

#include <string>

class OLOLORD_EXPORT ActionRoute : public AbstractRoute
{
private:
    typedef void (ActionRoute::*HandleActionFunction)(const QString &action, const Tools::PostParameters &params,
                                                      const Translator::Qt &tq);
    typedef QMap<QString, HandleActionFunction> HandleActionMap;
public:
    explicit ActionRoute(cppcms::application &app);
public:
    static QStringList availableActions();
public:
    void handle(std::string action);
    unsigned int handlerArgumentCount() const;
    std::string key() const;
    int priority() const;
    std::string regex() const;
    std::string url() const;
private:
    static HandleActionMap actionMap();
private:
    void handleAddFile(const QString &action, const Tools::PostParameters &params, const Translator::Qt &tq);
    void handleBanPoster(const QString &action, const Tools::PostParameters &params, const Translator::Qt &tq);
    void handleBanUser(const QString &action, const Tools::PostParameters &params, const Translator::Qt &tq);
    void handleChangeLocale(const QString &action, const Tools::PostParameters &params, const Translator::Qt &tq);
    void handleChangeSettings(const QString &action, const Tools::PostParameters &params, const Translator::Qt &tq);
    void handleCreatePost(const QString &action, const Tools::PostParameters &params, const Translator::Qt &tq);
    void handleCreateThread(const QString &action, const Tools::PostParameters &params, const Translator::Qt &tq);
    void handleDelall(const QString &action, const Tools::PostParameters &params, const Translator::Qt &tq);
    void handleDeleteFile(const QString &action, const Tools::PostParameters &params, const Translator::Qt &tq);
    void handleDeletePost(const QString &action, const Tools::PostParameters &params, const Translator::Qt &tq);
    void handleEditAudioTags(const QString &action, const Tools::PostParameters &params, const Translator::Qt &tq);
    void handleEditPost(const QString &action, const Tools::PostParameters &params, const Translator::Qt &tq);
    void handleLogin(const QString &action, const Tools::PostParameters &params, const Translator::Qt &tq);
    void handleLogout(const QString &action, const Tools::PostParameters &params, const Translator::Qt &tq);
    void handleMoveThread(const QString &action, const Tools::PostParameters &params, const Translator::Qt &tq);
    void handleSetThreadFixed(const QString &action, const Tools::PostParameters &params, const Translator::Qt &tq);
    void handleSetThreadOpened(const QString &action, const Tools::PostParameters &params, const Translator::Qt &tq);
    void handleSetVoteOpened(const QString &action, const Tools::PostParameters &params, const Translator::Qt &tq);
    void handleUnvote(const QString &action, const Tools::PostParameters &params, const Translator::Qt &tq);
    void handleVote(const QString &action, const Tools::PostParameters &params, const Translator::Qt &tq);
    void redirect(const QString &path = QString());
    void setCookie(const QString &name, const QString &sourceName, const Tools::PostParameters &params);
    bool testBoard(AbstractBoard *board, const QString &action, const QString &logTarget, const Translator::Qt &tq);
};

#endif // ACTIONROUTE_H
