#include "rpgboard.h"

#include "controller/rpgboard.h"
#include "controller/rpgthread.h"
#include "database.h"
#include "stored/thread.h"
#include "translator.h"

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

void rpgBoard::beforeStoring(Post *post, const Tools::PostParameters &params, bool thread)
{
    if (!post)
        return;
    if (!thread && !Database::isOp(post->board(), post->thread()->number(), post->posterIp(), post->hashpass()))
        return;
    bool ok = false;
    int count = params.value("voteVariantCount").toInt(&ok);
    if (!ok || count <= 0)
        return;
    QVariantMap m;
    QVariantList list;
    foreach (int i, bRangeD(1, count)) {
        QVariantMap mm;
        QString text = params.value("voteVariant" + QString::number(i));
        if (text.isEmpty())
            continue;
        mm.insert("text", text);
        list << mm;
    }
    if (list.isEmpty())
        return;
    m.insert("variants", list);
    m.insert("multiple", params.value("multipleVoteVariants") == "true");
    m.insert("text", params.value("voteText"));
    post->setUserData(m);
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
    bool en = true;
    unsigned int ip = Tools::ipNum(Tools::userIp(req));
    foreach (const QVariant &v, m.value("users").toList()) {
        if (v.toUInt() == ip) {
            en = false;
            break;
        }
    }
    o["voteEnabled"] = en;
    cppcms::json::array arr;
    foreach (const QVariant &v, m.value("variants").toList()) {
        QVariantMap mm = v.toMap();
        cppcms::json::object oo;
        oo["text"] = Tools::toStd(mm.value("text").toString());
        oo["voteCount"] = mm.value("voteCount").toUInt();
        arr.push_back(oo);
    }
    o["voteVariants"] = arr;
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
    cc->multipleVoteVariantsText = ts.translate("rpgBoard", "Multiple variants allowed:", "multipleVoteVariantsText");
    cc->removeVoteVariantText = ts.translate("rpgBoard", "Remove variant", "removeVoteVariantText");
    cc->postFormLabelVote = ts.translate("rpgBoard", "Vote:", "postFormLabelVote");
    cc->userIp = Tools::ipNum(Tools::userIp(req));
    cc->voteActionText = ts.translate("rpgBoard", "Vote", "voteActionText");
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
    cc->multipleVoteVariantsText = ts.translate("rpgBoard", "Multiple variants allowed:", "multipleVoteVariantsText");
    cc->removeVoteVariantText = ts.translate("rpgBoard", "Remove variant", "removeVoteVariantText");
    cc->postFormLabelVote = ts.translate("rpgBoard", "Vote:", "postFormLabelVote");
    cc->userIp = Tools::ipNum(Tools::userIp(req));
    cc->voteEnabled = Database::isOp("rpg", quint64(cc->number), Tools::userIp(req), Tools::hashpass(req));
    cc->voteActionText = ts.translate("rpgBoard", "Vote", "voteActionText");
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
