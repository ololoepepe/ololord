#include "rpgboard.h"

#include "controller/rpgboard.h"
#include "controller/rpgthread.h"
#include "database.h"
#include "stored/thread.h"
#include "translator.h"

#include <BUuid>

#include <QDebug>
#include <QLocale>
#include <QString>
#include <QVariant>
#include <QVariantList>
#include <QVariantMap>

#include <cppcms/http_request.h>
#include <cppcms/json.h>

rpgBoard::rpgBoard()
{
    //
}

bool rpgBoard::beforeStoringEditedPost(const cppcms::http::request &req, cppcms::json::value &userData, Post &post,
                                       Thread &thread, QString *error)
{
    try {
        if (userData.is_null() || userData.is_undefined())
            return bRet(error, QString(), true);
        TranslatorQt tq(req);
        cppcms::json::object o = userData.object();
        QVariantMap m = post.userData().toMap();
        cppcms::json::array arr = o["variants"].array();
        QVariantList variants = m.value("variants").toList();
        QStringList ids;
        for (int i = 0; i < int(arr.size()); ++i) {
            cppcms::json::object oo = arr.at(i).object();
            QString id = Tools::fromStd(oo["id"].str());
            QString text = Tools::fromStd(oo["text"].str());
            if (text.isEmpty())
                continue;
            if (!id.isEmpty()) {
                bool found = false;
                for (int i = 0; i < variants.size(); ++i) {
                    QVariantMap mm = variants.at(i).toMap();
                    if (mm.value("id").toString() != id)
                        continue;
                    mm["text"] = text;
                    variants[i] = mm;
                    found = true;
                }
                if (!found)
                    return bRet(error, tq.translate("vote", "Invalid vote", "error"), false);
            } else {
                QVariantMap mm;
                mm.insert("text", text);
                id = BUuid::createUuid().toString(true);
                mm.insert("id", id);
                variants << mm;
            }
            ids << id;
        }
        for (int i = 0; i < variants.size(); ++i) {
            if (!ids.contains(variants.at(i).toMap().value("id").toString()))
                variants.removeAt(i);
        }
        if (variants.isEmpty()) {
            post.setUserData(QVariant());
            return bRet(error, QString(), true);
        }
        QString text = Tools::fromStd(o["text"].str());
        if ((!thread.number() == post.number()
             && !Database::isOp(post.board(), thread.number(), post.posterIp(), post.hashpass()))
                && (m["variants"] != variants || m["multiple"] != o["multiple"].boolean() || m["text"] != text)) {
            return bRet(error, tq.translate("rpgBoard", "Attempt to edit voting while not being the OP", "error"), false);
        }
        m["variants"] = variants;
        m["multiple"] = o["multiple"].boolean();
        m["text"] = text;
        post.setUserData(m);
        return bRet(error, QString(), true);
    } catch (const cppcms::json::bad_value_cast &e) {
        return bRet(error, Tools::fromStd(e.what()), false);
    } catch (const std::out_of_range &e) {
        return bRet(error, Tools::fromStd(e.what()), false);
    }
}

bool rpgBoard::beforeStoringNewPost(const cppcms::http::request &req, Post *post, const Tools::PostParameters &params,
                                    bool thread, QString *error, QString *description)
{
    TranslatorQt tq(req);
    if (!post) {
        return bRet(error, tq.translate("rpgBoard", "Internal error", "error"), description,
                    tq.translate("rpgBoard", "Internal logic error", "description"), false);
    }
    bool ok = false;
    int count = params.value("voteVariantCount").toInt(&ok);
    if (!ok || count <= 0)
        return bRet(error, QString(), description, QString(), true);
    if (!thread && !Database::isOp(post->board(), post->thread()->number(), post->posterIp(), post->hashpass())) {
        return bRet(error, tq.translate("rpgBoard", "Not enough rights", "error"), description,
                    tq.translate("rpgBoard", "Attempt to attach voting while not being the OP", "description"), false);
    }
    QVariantMap m;
    QVariantList variants;
    foreach (int i, bRangeD(1, count)) {
        QVariantMap mm;
        QString text = params.value("voteVariant" + QString::number(i));
        if (text.isEmpty())
            continue;
        mm.insert("text", text);
        mm.insert("id", BUuid::createUuid().toString(true));
        variants << mm;
    }
    if (variants.isEmpty())
        return bRet(error, QString(), description, QString(), true);
    m.insert("variants", variants);
    m.insert("multiple", params.value("multipleVoteVariants") == "true");
    m.insert("text", params.value("voteText"));
    post->setUserData(m);
    return bRet(error, QString(), description, QString(), true);
}

QString rpgBoard::name() const
{
    return "rpg";
}

QString rpgBoard::title(const QLocale &l) const
{
    return TranslatorQt(l).translate("rpgBoard", "Role-playing games", "board title");
}

cppcms::json::object rpgBoard::toJson(const Content::Post &post, const cppcms::http::request &req) const
{
    cppcms::json::object o = AbstractBoard::toJson(post, req);
    QVariantMap m = post.userData.toMap();
    bool voted = false;
    unsigned int ip = Tools::ipNum(Tools::userIp(req));
    cppcms::json::array arr;
    QVariantList variants = m.value("variants").toList();
    foreach (const QVariant &v, variants) {
        QVariantMap mm = v.toMap();
        cppcms::json::object oo;
        oo["id"] = Tools::toStd(mm.value("id").toString());
        oo["text"] = Tools::toStd(mm.value("text").toString());
        oo["voteCount"] = mm.value("voteCount").toUInt();
        foreach (const QVariant &v, mm.value("users").toList()) {
            if (v.toUInt() == ip) {
                oo["selected"] = true;
                voted = true;
                break;
            }
        }
        arr.push_back(oo);
    }
    o["voteVoted"] = voted;
    o["voteVariants"] = arr;
    o["voteDisabled"] = m.value("disabled").toBool();
    o["voteMultiple"] = m.value("multiple").toBool();
    o["voteText"] = Tools::toStd(m.value("text").toString());
    return o;
}

void rpgBoard::beforeRenderBoard(const cppcms::http::request &req, Content::Board *c)
{
    Content::rpgBoard *cc = dynamic_cast<Content::rpgBoard *>(c);
    if (!cc)
        return;
    TranslatorStd ts(req);
    cc->addVoteVariantText = ts.translate("rpgBoard", "Add variant", "addVoteVariantText");
    cc->closeVoteActionText = ts.translate("rpgBoard", "Close voting", "closeVoteActionText");
    cc->multipleVoteVariantsText = ts.translate("rpgBoard", "Multiple variants allowed:", "multipleVoteVariantsText");
    cc->openVoteActionText = ts.translate("rpgBoard", "Open voting", "openVoteActionText");
    cc->removeVoteVariantText = ts.translate("rpgBoard", "Remove variant", "removeVoteVariantText");
    cc->postFormLabelVote = ts.translate("rpgBoard", "Vote:", "postFormLabelVote");
    cc->unvoteActionText = ts.translate("rpgBoard", "Take vote back", "unvoteActionText");
    cc->userIp = Tools::ipNum(Tools::userIp(req));
    cc->voteActionText = ts.translate("rpgBoard", "Vote", "voteActionText");
    cc->voteClosedText = ts.translate("rpgBoard", "Voting is closed", "voteClosedText");
    cc->votedText = ts.translate("rpgBoard", "voted:", "votedText");
    cc->voteTextText = ts.translate("rpgBoard", "Text:", "voteTextText");
}

void rpgBoard::beforeRenderThread(const cppcms::http::request &req, Content::Thread *c)
{
    Content::rpgThread *cc = dynamic_cast<Content::rpgThread *>(c);
    if (!cc)
        return;
    TranslatorStd ts(req);
    cc->addVoteVariantText = ts.translate("rpgBoard", "Add variant", "addVoteVariantText");
    cc->closeVoteActionText = ts.translate("rpgBoard", "Close voting", "closeVoteActionText");
    cc->multipleVoteVariantsText = ts.translate("rpgBoard", "Multiple variants allowed:", "multipleVoteVariantsText");
    cc->openVoteActionText = ts.translate("rpgBoard", "Open voting", "openVoteActionText");
    cc->removeVoteVariantText = ts.translate("rpgBoard", "Remove variant", "removeVoteVariantText");
    cc->postFormLabelVote = ts.translate("rpgBoard", "Vote:", "postFormLabelVote");
    cc->unvoteActionText = ts.translate("rpgBoard", "Take vote back", "unvoteActionText");
    cc->userIp = Tools::ipNum(Tools::userIp(req));
    cc->voteEnabled = Database::isOp("rpg", quint64(cc->number), Tools::userIp(req), Tools::hashpass(req));
    cc->voteActionText = ts.translate("rpgBoard", "Vote", "voteActionText");
    cc->voteClosedText = ts.translate("rpgBoard", "Voting is closed", "voteClosedText");
    cc->votedText = ts.translate("rpgBoard", "voted:", "votedText");
    cc->voteTextText = ts.translate("rpgBoard", "Text:", "voteTextText");
}

Content::Board *rpgBoard::createBoardController(const cppcms::http::request &/*req*/, QString &viewName)
{
    viewName = "rpg_board";
    return new Content::rpgBoard;
}

Content::Thread *rpgBoard::createThreadController(const cppcms::http::request &/*req*/, QString &viewName)
{
    viewName = "rpg_thread";
    return new Content::rpgThread;
}
