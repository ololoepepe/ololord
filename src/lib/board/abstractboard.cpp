#include "abstractboard.h"

#include "board/aboard.h"
#include "board/bboard.h"
#include "board/cgboard.h"
#include "board/hboard.h"
#include "board/mlpboard.h"
#include "board/prboard.h"
#include "board/rfboard.h"
#include "board/threedpdboard.h"
#include "board/vgboard.h"
#include "controller/baseboard.h"
#include "controller/board.h"
#include "controller/controller.h"
#include "controller/rules.h"
#include "controller/thread.h"
#include "database.h"
#include "plugin/global/boardfactoryplugininterface.h"
#include "settingslocker.h"
#include "stored/postcounter.h"
#include "stored/postcounter-odb.hxx"
#include "stored/thread.h"
#include "stored/thread-odb.hxx"
#include "tools.h"
#include "translator.h"

#include <BCoreApplication>
#include <BDirTools>
#include <BeQt>
#include <BPluginInterface>
#include <BPluginWrapper>
#include <BSettingsNode>
#include <BTerminal>
#include <BTranslation>

#include <QCryptographicHash>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QList>
#include <QMap>
#include <QMutex>
#include <QMutexLocker>
#include <QSet>
#include <QSettings>
#include <QSharedPointer>
#include <QString>
#include <QStringList>
#include <QtAlgorithms>
#include <QVariant>

#include <cppcms/application.h>
#include <cppcms/http_request.h>

#include <odb/database.hxx>
#include <odb/connection.hxx>
#include <odb/qt/lazy-ptr.hxx>
#include <odb/query.hxx>
#include <odb/result.hxx>
#include <odb/transaction.hxx>

#include <ostream>
#include <string>
#include <vector>

#include <fstream>

static bool threadLessThan(const Thread &t1, const Thread &t2)
{
    if (t1.fixed() == t2.fixed())
        return t1.dateTime().toUTC() > t2.dateTime().toUTC();
    else if (t1.fixed())
        return true;
    else
        return false;
}

QMap<QString, AbstractBoard *> AbstractBoard::boards;
bool AbstractBoard::boardsInitialized = false;
QMutex AbstractBoard::boardsMutex(QMutex::Recursive);

AbstractBoard::AbstractBoard()
{
    //
}

AbstractBoard::~AbstractBoard()
{
    //
}

AbstractBoard *AbstractBoard::board(const QString &name)
{
    initBoards();
    boardsMutex.lock();
    AbstractBoard *b = boards.value(name);
    boardsMutex.unlock();
    return (b && b->isEnabled()) ? b : 0;
}

AbstractBoard::BoardInfoList AbstractBoard::boardInfos(const QLocale &l, bool includeHidden)
{
    initBoards();
    BoardInfoList list;
    boardsMutex.lock();
    foreach (const QString &key, boards.keys()) {
        AbstractBoard *board = boards.value(key);
        if (!board || !board->isEnabled() || (!includeHidden && board->isHidden()))
            continue;
        BoardInfo info;
        info.name = Tools::toStd(board->name());
        info.title = Tools::toStd(boards.value(key)->title(l));
        list.push_back(info);
    }
    boardsMutex.unlock();
    return list;
}

QStringList AbstractBoard::boardNames(bool includeHidden)
{
    initBoards();
    boardsMutex.lock();
    QStringList list = boards.keys();
    boardsMutex.unlock();
    foreach (int i, bRangeR(list.size() - 1, 0)) {
        AbstractBoard *board = boards.value(list.at(i));
        if (!board || !board->isEnabled() || (!includeHidden && board->isHidden()))
            list.removeAt(i);
    }
    return list;
}

void AbstractBoard::reloadBoards()
{
    initBoards(true);
}

unsigned int AbstractBoard::archiveLimit() const
{
    SettingsLocker s;
    return s->value("Board/" + name() + "/archive_limit", s->value("Board/archive_limit", 0)).toUInt();
}

QString AbstractBoard::bannerFileName() const
{
    QString path = BDirTools::findResource("static/img/banner", BDirTools::AllResources);
    if (path.isEmpty())
        return "";
    QStringList fns = QDir(path).entryList(QStringList() << (name() + "*.*"), QDir::Files);
    if (fns.isEmpty())
        return "";
    qsrand((uint) QDateTime::currentMSecsSinceEpoch());
    return fns.at(qrand() % fns.size());
}

unsigned int AbstractBoard::bumpLimit() const
{
    SettingsLocker s;
    return s->value("Board/" + name() + "/bump_limit", s->value("Board/bump_limit", 500)).toUInt();
}

void AbstractBoard::createPost(cppcms::application &app)
{
    Tools::log(app, "Handling post creation");
    cppcms::http::request &req = app.request();
    if (!Controller::testBan(app, Controller::WriteAction, name()))
        return;
    Tools::PostParameters params = Tools::postParameters(req);
    Tools::FileList files = Tools::postFiles(req);
    if (!Controller::testParams(app, params, files, name()))
        return;
    TranslatorQt tq(req);
    if (!Tools::postingEnabled(name())) {
        return Controller::renderError(app, tq.translate("AbstractBoard", "Posting disabled", "error"),
            tq.translate("AbstractBoard", "Posting is disabled for this board","description"));
    }
    QString err;
    QString desc;
    Database::CreatePostParameters p(req, params, files, tq.locale());
    p.bumpLimit = bumpLimit();
    p.postLimit = postLimit();
    p.error = &err;
    p.description = &desc;
    if (!Database::createPost(p))
        return Controller::renderError(app, err, desc);
    quint64 threadNumber = Tools::postParameters(req).value("thread").toULongLong();
    Controller::redirect(app, "/board/" + name() + "/thread/" + QString::number(threadNumber) + ".html");
    Tools::log(app, "Handled post creation");
}

void AbstractBoard::createThread(cppcms::application &app)
{
    Tools::log(app, "Handling thread creation");
    cppcms::http::request &req = app.request();
    if (!Controller::testBan(app, Controller::WriteAction, name()))
        return;
    Tools::PostParameters params = Tools::postParameters(req);
    Tools::FileList files = Tools::postFiles(req);
    if (!Controller::testParams(app, params, files, name()))
        return;
    TranslatorQt tq(req);
    if (!Tools::postingEnabled(name())) {
        return Controller::renderError(app, tq.translate("AbstractBoard", "Posting disabled", "error"),
            tq.translate("AbstractBoard", "Posting is disabled for this board", "description"));
    }
    QString err;
    QString desc;
    Database::CreateThreadParameters p(req, params, files, tq.locale());
    p.archiveLimit = archiveLimit();
    p.threadLimit = threadLimit();
    p.error = &err;
    p.description = &desc;
    quint64 threadNumber = Database::createThread(p);
    if (!threadNumber)
        return Controller::renderError(app, err, desc);
    Controller::redirect(app, "/board/" + name() + "/thread/" + QString::number(threadNumber) + ".html");
    Tools::log(app, "Handled thread creation");
}

void AbstractBoard::handleBoard(cppcms::application &app, unsigned int page)
{
    Tools::log(app, "Handling board");
    if (!Controller::testBan(app, Controller::ReadAction, name()))
        return;
    Content::Board c;
    TranslatorQt tq(app.request());
    TranslatorStd ts(app.request());
    unsigned int pageCount = 0;
    bool postingEn = postingEnabled();
    try {
        odb::database *db = Database::createConnection();
        if (!db) {
            return Controller::renderError(app, tq.translate("AbstractBoard", "Internal error", "error"),
                                           tq.translate("AbstractBoard", "Internal database error", "description"));
        }
        odb::transaction transaction(db->begin());
        odb::result<Thread> r(db->query<Thread>(odb::query<Thread>::board == name()
                                                && odb::query<Thread>::archived == false));
        QList<Thread> list;
        for (odb::result<Thread>::iterator i = r.begin(); i != r.end(); ++i)
            list << *i;
        qSort(list.begin(), list.end(), &threadLessThan);
        pageCount = (list.size() / threadsPerPage()) + ((list.size() % threadsPerPage()) ? 1 : 0);
        if (!pageCount)
            pageCount = 1;
        if (page >= pageCount)
            return Controller::renderNotFound(app);
        foreach (const Thread &tt, list.mid(page * threadsPerPage(), threadsPerPage())) {
            Content::Board::Thread thread;
            const Thread::Posts &posts = tt.posts();
            thread.bumpLimit = bumpLimit();
            thread.fixed = tt.fixed();
            thread.postLimit = postLimit();
            thread.postCount = posts.size();
            thread.postingEnabled = postingEn && tt.postingEnabled();
            thread.opPost = Controller::toController(*posts.first().load(), name(), tt.number(), ts.locale(),
                                                     app.request(), processCode());
            foreach (int i, bRangeR(posts.size() - 1, posts.size() - 3)) {
                if (i <= 0)
                    break;
                thread.lastPosts.push_front(Controller::toController(*posts.at(i).load(), name(), tt.number(),
                                                                     ts.locale(), app.request(), processCode()));
            }
            c.threads.push_back(thread);
        }
        transaction.commit();
    }  catch (const odb::exception &e) {
        return Controller::renderError(app, tq.translate("AbstractBoard", "Internal error", "error"),
                                       Tools::fromStd(e.what()));
    }
    Controller::initBaseBoard(c, app.request(), this, postingEn, title(ts.locale()));
    c.boardRulesLinkText = ts.translate("AbstractBoard", "Borad rules", "boardRulesLinkText");
    c.currentPage = page;
    c.omittedPostsText = ts.translate("AbstractBoard", "Posts omitted:", "omittedPostsText");
    foreach (int i, bRangeD(0, pageCount - 1))
        c.pages.push_back(i);
    c.toNextPageText = ts.translate("AbstractBoard", "Next page", "toNextPageText");
    c.toPreviousPageText = ts.translate("AbstractBoard", "Previous page", "toPreviousPageText");
    c.toThread = ts.translate("AbstractBoard", "Answer", "toThread");
    app.render("board", c);
    Tools::log(app, "Handled board");
}

void AbstractBoard::handleRules(cppcms::application &app)
{
    Tools::log(app, "Handling rules");
    Content::Rules c;
    TranslatorQt tq(app.request());
    TranslatorStd ts(app.request());
    QString pageTitle = title(tq.locale()) + " - " + tq.translate("AbstractBoard", "rules", "pageTitle");
    Controller::initBase(c, app.request(), pageTitle);
    c.currentBoard.name = Tools::toStd(name());
    c.currentBoard.title = Tools::toStd(title(tq.locale()));
    c.noRulesText = ts.translate("AbstractBoard", "There are no specific rules for this board.", "noRulesText");;
    foreach (const QString &r, rules(ts.locale()))
        c.rules.push_back(Tools::toStd(Controller::toHtml(r)));
    app.render("rules", c);
    Tools::log(app, "Handled rules");
}

void AbstractBoard::handleThread(cppcms::application &app, quint64 threadNumber)
{
    Tools::log(app, "Handling thread");
    if (!Controller::testBan(app, Controller::ReadAction, name()))
        return;
    Content::Thread c;
    TranslatorQt tq(app.request());
    TranslatorStd ts(app.request());
    bool postingEn = false;
    QString pageTitle;
    try {
        odb::database *db = Database::createConnection();
        if (!db) {
            return Controller::renderError(app, tq.translate("AbstractBoard", "Internal error", "error"),
                                           tq.translate("AbstractBoard", "Internal database error", "description"));
        }
        odb::transaction transaction(db->begin());
        odb::result<Thread> r(db->query<Thread>(odb::query<Thread>::board == name()
                                                && odb::query<Thread>::archived == false));
        bool threadFound = false;
        for (odb::result<Thread>::iterator i = r.begin(); i != r.end(); ++i) {
            if (i->number() == threadNumber) {
                const Thread::Posts posts = i->posts();
                if (posts.isEmpty()) {
                    return Controller::renderError(app, tq.translate("AbstractBoard", "Internal error", "error"),
                        tq.translate("AbstractBoard", "Internal database error", "description"));
                }
                c.fixed = i->fixed();
                pageTitle = posts.first().load()->subject();
                postingEn = Tools::postingEnabled(name()) && i->postingEnabled();
                c.opPost = Controller::toController(*posts.first().load(), name(), i->number(), ts.locale(),
                                                    app.request(), processCode());
                foreach (int j, bRangeD(1, posts.size() - 1)) {
                    c.posts.push_back(Controller::toController(*posts.at(j).load(), name(), i->number(), ts.locale(),
                                                               app.request(), processCode()));
                }
                threadFound = true;
                break;
            }
        }
        if (!threadFound)
            return Controller::renderNotFound(app);
        transaction.commit();
    }  catch (const odb::exception &e) {
        return Controller::renderError(app, tq.translate("AbstractBoard", "Internal error", "error"),
                                       Tools::fromStd(e.what()));
    }
    Controller::initBaseBoard(c, app.request(), this, postingEn, pageTitle, threadNumber);
    c.bumpLimit = bumpLimit();
    c.postLimit = postLimit();
    app.render("thread", c);
    Tools::log(app, "Handled thread");
}

bool AbstractBoard::isEnabled() const
{
    return SettingsLocker()->value("Board/" + name() + "/enabled", true).toBool();
}

bool AbstractBoard::isHidden() const
{
    return SettingsLocker()->value("Board/" + name() + "/hidden", false).toBool();
}

bool AbstractBoard::postingEnabled() const
{
    SettingsLocker s;
    return s->value("Board/" + name() + "/posting_enabled", s->value("Board/posting_enabled", true)).toBool();
}

unsigned int AbstractBoard::postLimit() const
{
    SettingsLocker s;
    return s->value("Board/" + name() + "/post_limit", s->value("Board/post_limit", 1000)).toUInt();
}

bool AbstractBoard::processCode() const
{
    return false;
}

QStringList AbstractBoard::rules(const QLocale &l) const
{
    return Tools::rules("rules", l) + Tools::rules("rules/" + name(), l);
}

unsigned int AbstractBoard::threadLimit() const
{
    SettingsLocker s;
    return s->value("Board/" + name() + "/thread_limit", s->value("Board/thread_limit", 200)).toUInt();
}

unsigned int AbstractBoard::threadsPerPage() const
{
    SettingsLocker s;
    return s->value("Board/" + name() + "/threads_per_page", s->value("Board/threads_per_page", 20)).toUInt();
}

void AbstractBoard::cleanupBoards()
{
    QMutexLocker locker(&boardsMutex);
    foreach (AbstractBoard *b, boards)
        delete b;
    boards.clear();
}

void AbstractBoard::initBoards(bool reinit)
{
    QMutexLocker locker(&boardsMutex);
    if (boardsInitialized && !reinit)
        return;
    if (reinit) {
        QSet<QString> boardNames = QSet<QString>::fromList(boards.keys());
        BSettingsNode *n = BTerminal::rootSettingsNode()->find("Board");
        foreach (BSettingsNode *nn, n->childNodes()) {
            if (!boardNames.contains(nn->key()))
                continue;
            n->removeChild(nn);
            delete nn;
        }
        foreach (AbstractBoard *b, boards.values())
            delete b;
        boards.clear();
    }
    AbstractBoard *b = new aBoard;
    boards.insert(b->name(), b);
    b = new bBoard;
    boards.insert(b->name(), b);
    b = new cgBoard;
    boards.insert(b->name(), b);
    b = new hBoard;
    boards.insert(b->name(), b);
    b = new mlpBoard;
    boards.insert(b->name(), b);
    b = new prBoard;
    boards.insert(b->name(), b);
    b = new rfBoard;
    boards.insert(b->name(), b);
    b = new threedpdBoard;
    boards.insert(b->name(), b);
    b = new vgBoard;
    boards.insert(b->name(), b);
    foreach (BPluginWrapper *pw, BCoreApplication::pluginWrappers("board-factory")) {
        pw->unload();
        BCoreApplication::removePlugin(pw);
    }
    BCoreApplication::loadPlugins(QStringList() << "board-factory");
    foreach (BPluginWrapper *pw, BCoreApplication::pluginWrappers("board-factory")) {
        BoardFactoryPluginInterface *i = qobject_cast<BoardFactoryPluginInterface *>(pw->instance());
        if (!i)
            continue;
        foreach (AbstractBoard *b, i->createBoards()) {
            if (!b)
                continue;
            QString nm = b->name();
            if (nm.isEmpty()) {
                delete b;
                continue;
            }
            if (boards.contains(nm))
                delete boards.take(nm);
            boards.insert(nm, b);
        }
    }
    BSettingsNode *n = BTerminal::rootSettingsNode()->find("Board");
    foreach (const QString &boardName, boards.keys()) {
        BSettingsNode *nn = new BSettingsNode(boardName, n);
        BSettingsNode *nnn = new BSettingsNode(QVariant::Bool, "captcha_enabled", nn);
        nnn->setDescription(BTranslation::translate("AbstractBoard", "Determines if captcha is enabled on this board.\n"
                                                    "The default is true."));
        nnn = new BSettingsNode(QVariant::UInt, "threads_per_page", nn);
        nnn->setDescription(BTranslation::translate("AbstractBoard", "Number of threads per one page on this board.\n"
                                                    "The default is 20."));
        nnn = new BSettingsNode(QVariant::Bool, "posting_enabled", nn);
        nnn->setDescription(BTranslation::translate("AbstractBoard", "Determines if posting is enabled on this board.\n"
                                                    "The default is true."));
        nnn = new BSettingsNode(QVariant::UInt, "bump_limit", nn);
        nnn->setDescription(BTranslation::translate("AbstractBoard", "Maximum bump count on this board.\n"
                                                    "When a thread has reached it's bump limit, "
                                                    "it will not be raised anymore.\n"
                                                    "The default is 500."));
        nnn = new BSettingsNode(QVariant::UInt, "post_limit", nn);
        nnn->setDescription(BTranslation::translate("AbstractBoard", "Maximum post count per thread on this board.\n"
                                                    "The default is 1000."));
        nnn = new BSettingsNode(QVariant::UInt, "thread_limit", nn);
        nnn->setDescription(BTranslation::translate("AbstractBoard", "Maximum thread count for this board.\n"
                                                    "When the limit is reached, the most old threads get deleted.\n"
                                                    "The default is 200."));
        nnn = new BSettingsNode(QVariant::Bool, "hidden", nn);
        nnn->setDescription(BTranslation::translate("AbstractBoard", "Determines if this board is hidden.\n"
                                                    "A hidden board will not appear in navigation bars.\n"
                                                    "The default is false."));
        nnn = new BSettingsNode(QVariant::Bool, "enabled", nn);
        nnn->setDescription(BTranslation::translate("AbstractBoard", "Determines if this board is enabled.\n"
                                                    "A disabled board will not be accessible by any means.\n"
                                                    "The default is true."));
        nnn = new BSettingsNode(QVariant::UInt, "max_email_length", nn);
        nnn->setDescription(BTranslation::translate("AbstractBoard",
                                                    "Maximum length of the e-mail field for this board.\n"
                                                    "The default is 150."));
        nnn = new BSettingsNode(QVariant::UInt, "max_name_length", nn);
        nnn->setDescription(BTranslation::translate("AbstractBoard",
                                                    "Maximum length of the name field for this board.\n"
                                                    "The default is 50."));
        nnn = new BSettingsNode(QVariant::UInt, "max_subject_length", nn);
        nnn->setDescription(BTranslation::translate("AbstractBoard",
                                                    "Maximum length of the subject field for this board.\n"
                                                    "The default is 150."));
        nnn = new BSettingsNode(QVariant::UInt, "max_text_length", nn);
        nnn->setDescription(BTranslation::translate("AbstractBoard",
                                                    "Maximum length of the text field for this board.\n"
                                                    "The default is 15000."));
        nnn = new BSettingsNode(QVariant::UInt, "max_password_length", nn);
        nnn->setDescription(BTranslation::translate("AbstractBoard",
                                                    "Maximum length of the password field for this board.\n"
                                                    "The default is 150."));
        nnn = new BSettingsNode(QVariant::UInt, "max_file_size", nn);
        nnn->setDescription(BTranslation::translate("AbstractBoard",
                                                    "Maximum attached file size (in bytes) for this board.\n"
                                                    "The default is 10485760 (10 MB)."));
        nnn = new BSettingsNode(QVariant::UInt, "max_file_count", nn);
        nnn->setDescription(BTranslation::translate("AbstractBoard",
                                                    "Maximum attached file count for this board.\n"
                                                    "The default is 1."));
        nnn = new BSettingsNode(QVariant::UInt, "archive_limit", nn);
        nnn->setDescription(BTranslation::translate("AbstractBoard", "Maximum archived thread count for this board.\n"
                                                   "The default is 0 (do not archive)."));
    }
    if (!reinit)
        qAddPostRoutine(&cleanupBoards);
    boardsInitialized = true;
}
