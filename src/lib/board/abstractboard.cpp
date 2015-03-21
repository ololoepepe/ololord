#include "abstractboard.h"

#include "board/aboard.h"
#include "board/bboard.h"
#include "board/cgboard.h"
#include "board/echoboard.h"
#include "board/hboard.h"
#include "board/intboard.h"
#include "board/mlpboard.h"
#include "board/prboard.h"
#include "board/rfboard.h"
#include "board/socboard.h"
#include "board/threedpdboard.h"
#include "board/vgboard.h"
#include "cache.h"
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
#include "transaction.h"
#include "translator.h"

#include <BCoreApplication>
#include <BDirTools>
#include <BeQt>
#include <BPluginInterface>
#include <BPluginWrapper>
#include <BSettingsNode>
#include <BTerminal>
#include <BTranslation>

#include <QBuffer>
#include <QByteArray>
#include <QCryptographicHash>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QImage>
#include <QList>
#include <QMap>
#include <QMutex>
#include <QMutexLocker>
#include <QScopedPointer>
#include <QSet>
#include <QSettings>
#include <QSharedPointer>
#include <QString>
#include <QStringList>
#include <QtAlgorithms>
#include <QVariant>
#include <QVariantMap>

#include <cppcms/application.h>
#include <cppcms/http_request.h>

#include <odb/database.hxx>
#include <odb/connection.hxx>
#include <odb/qt/lazy-ptr.hxx>
#include <odb/query.hxx>
#include <odb/result.hxx>
#include <odb/transaction.hxx>

#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>

#include <ostream>
#include <string>
#include <vector>

#include <fstream>

class FileTransaction
{
private:
    bool commited;
    QStringList mfileNames;
public:
    explicit FileTransaction()
    {
        commited = false;
    }
    ~FileTransaction()
    {
        if (commited)
            return;
        foreach (const QString &fn, mfileNames)
            QFile::remove(fn);
    }
public:
    void commit()
    {
        commited = true;
    }
    QStringList fileNames() const
    {
        return mfileNames;
    }
    void addFile(const QString &fn)
    {
        mfileNames << fn;
    }
};

static void scaleThumbnail(QImage &img, QVariantMap &m)
{
    m.insert("height", img.height());
    m.insert("width", img.width());
    if (img.height() > 200 || img.width() > 200)
        img = img.scaled(200, 200, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    m.insert("thumbHeight", img.height());
    m.insert("thumbWidth", img.width());
}

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
const QString AbstractBoard::defaultFileTypes = "image/png,image/jpeg,image/gif,video/webm";

AbstractBoard::AbstractBoard() :
    captchaQuotaMutex(QMutex::Recursive)
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

unsigned int AbstractBoard::captchaQuota() const
{
    SettingsLocker s;
    return s->value("Board/" + name() + "/captcha_quota", s->value("Board/captcha_quota", 0)).toUInt();
}

unsigned int AbstractBoard::captchaQuota(const QString &ip) const
{
    if (ip.isEmpty())
        return 0;
    QMutexLocker locker(&captchaQuotaMutex);
    return captchaQuotaMap.value(ip);
}

void AbstractBoard::captchaSolved(const QString &ip)
{
    if (ip.isEmpty())
        return;
    QMutexLocker locker(&captchaQuotaMutex);
    captchaQuotaMap[ip] = captchaQuota();
}

void AbstractBoard::captchaUsed(const QString &ip)
{
    if (ip.isEmpty())
        return;
    QMutexLocker locker(&captchaQuotaMutex);
    unsigned int &q = captchaQuotaMap[ip];
    if (!q)
        captchaQuotaMap.remove(ip);
    else
        --q;
    if (!q)
        captchaQuotaMap.remove(ip);
}

void AbstractBoard::createPost(cppcms::application &app)
{
    Tools::log(app, "Handling post creation");
    cppcms::http::request &req = app.request();
    if (!Controller::testBan(app, Controller::WriteAction, name()))
        return;
    Tools::PostParameters params = Tools::postParameters(req);
    Tools::FileList files = Tools::postFiles(req);
    if (!Controller::testParams(this, app, params, files, true))
        return;
    TranslatorQt tq(req);
    if (!postingEnabled()) {
        return Controller::renderError(app, tq.translate("AbstractBoard", "Posting disabled", "error"),
            tq.translate("AbstractBoard", "Posting is disabled for this board","description"));
    }
    QString err;
    QString desc;
    beforeStoring(params, true);
    Database::CreatePostParameters p(req, params, files, tq.locale());
    p.bumpLimit = bumpLimit();
    p.postLimit = postLimit();
    p.error = &err;
    p.description = &desc;
    quint64 postNumber = 0L;
    if (!Database::createPost(p, &postNumber))
        return Controller::renderError(app, err, desc);
    Controller::renderSuccessfulPost(app, postNumber, p.referencedPosts);
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
    if (!Controller::testParams(this, app, params, files, false))
        return;
    TranslatorQt tq(req);
    if (!postingEnabled()) {
        return Controller::renderError(app, tq.translate("AbstractBoard", "Posting disabled", "error"),
            tq.translate("AbstractBoard", "Posting is disabled for this board", "description"));
    }
    QString err;
    QString desc;
    beforeStoring(params, false);
    Database::CreateThreadParameters p(req, params, files, tq.locale());
    p.archiveLimit = archiveLimit();
    p.threadLimit = threadLimit();
    p.error = &err;
    p.description = &desc;
    quint64 threadNumber = Database::createThread(p);
    if (!threadNumber)
        return Controller::renderError(app, err, desc);
    Controller::renderSuccessfulThread(app, threadNumber);
    Tools::log(app, "Handled thread creation");
}

QString AbstractBoard::defaultUserName(const QLocale &l) const
{
    return TranslatorQt(l).translate("AbstractBoard", "Anonymous", "defaultUserName");
}

void AbstractBoard::deleteFiles(const QStringList &fileNames)
{
    QString path = Tools::storagePath() + "/img/" + name();
    QFileInfo fi(path);
    if (!fi.exists() || !fi.isDir())
        return;
    foreach (const QString &fn, fileNames) {
        QFile::remove(path + "/" + fn);
        QFileInfo fii(fn);
        QString suff = !fii.suffix().compare("gif", Qt::CaseInsensitive) ? "png" : fii.suffix();
        QFile::remove(path + "/" + fii.baseName() + ".ololord-file-info");
        if (suff.compare("webm", Qt::CaseInsensitive))
            QFile::remove(path + "/" + fii.baseName() + "s." + suff);
        else
            QFile::remove(path + "/" + fii.baseName() + "s.png");
    }
}

void AbstractBoard::handleBoard(cppcms::application &app, unsigned int page)
{
    Tools::log(app, "Handling board: " + name());
    if (!Controller::testBan(app, Controller::ReadAction, name()))
        return;
    TranslatorQt tq(app.request());
    TranslatorStd ts(app.request());
    QString viewName;
    QScopedPointer<Content::Board> cc(createBoardController(app.request(), viewName));
    if (cc.isNull()) {
        return Controller::renderError(app, tq.translate("AbstractBoard", "Internal error", "error"),
                                       tq.translate("AbstractBoard", "Internal logic error", "description"));
    }
    if (viewName.isEmpty())
        viewName = "board";
    Content::Board &c = *cc;
    unsigned int pageCount = 0;
    bool postingEn = postingEnabled();
    try {
        Transaction t;
        if (!t) {
            return Controller::renderError(app, tq.translate("AbstractBoard", "Internal error", "error"),
                                           tq.translate("AbstractBoard", "Internal database error", "description"));
        }
        odb::query<Thread> q = odb::query<Thread>::board == name() && odb::query<Thread>::archived == false;
        QByteArray hashpass = Tools::hashpass(app.request());
        bool modOnBoard = Database::moderOnBoard(app.request(), name());
        QList<Thread> list = Database::query<Thread, Thread>(q);
        int lvl = Database::registeredUserLevel(app.request());
        foreach (int i, bRangeR(list.size() - 1, 0)) {
            Post opPost = *list.at(i).posts().first().load();
            if (opPost.premoderation() && opPost.hashpass() != hashpass
                    && (!modOnBoard || Database::registeredUserLevel(opPost.hashpass()) >= lvl)) {
                list.removeAt(i);
            }
        }
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
            thread.closed = !tt.postingEnabled();
            thread.fixed = tt.fixed();
            thread.postLimit = postLimit();
            thread.postCount = posts.size();
            thread.postingEnabled = postingEn && tt.postingEnabled();
            bool ok = false;
            QString err;
            thread.opPost = toController(*posts.first().load(), app.request(), &ok, &err);
            if (!ok)
                return Controller::renderError(app, tq.translate("AbstractBoard", "Internal error", "error"), err);
            unsigned int maxPosts = Tools::maxInfo(Tools::MaxLastPosts, name());
            foreach (int i, bRangeR(posts.size() - 1, 1)) {
                Post post = *posts.at(i).load();
                if (post.premoderation() && hashpass != post.hashpass()
                        && (!modOnBoard || Database::registeredUserLevel(post.hashpass()) >= lvl)) {
                    continue;
                }
                thread.lastPosts.push_front(toController(post, app.request(), &ok, &err));
                if (!ok)
                    return Controller::renderError(app, tq.translate("AbstractBoard", "Internal error", "error"), err);
                if (thread.lastPosts.size() >= maxPosts)
                    break;
            }
            thread.hidden = (Tools::cookieValue(app.request(), "postHidden" + name()
                                                + QString::number(tt.number())) == "true");
            c.threads.push_back(thread);
        }
        t.commit();
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
    beforeRenderBoard(app.request(), cc.data());
    app.render(Tools::toStd(viewName), c);
    Tools::log(app, "Handled board");
}

void AbstractBoard::handleRules(cppcms::application &app)
{
    Tools::log(app, "Handling rules: " + name());
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
    Tools::log(app, "Handling thread: " + name() + "/" + QString::number(threadNumber));
    if (!Controller::testBan(app, Controller::ReadAction, name()))
        return;
    TranslatorQt tq(app.request());
    TranslatorStd ts(app.request());
    QString viewName;
    QScopedPointer<Content::Thread> cc(createThreadController(app.request(), viewName));
    if (cc.isNull()) {
        return Controller::renderError(app, tq.translate("AbstractBoard", "Internal error", "error"),
                                       tq.translate("AbstractBoard", "Internal logic error", "description"));
    }
    if (viewName.isEmpty())
        viewName = "thread";
    Content::Thread &c = *cc;
    bool postingEn = postingEnabled();
    QString pageTitle;
    try {
        Transaction t;
        if (!t) {
            return Controller::renderError(app, tq.translate("AbstractBoard", "Internal error", "error"),
                                           tq.translate("AbstractBoard", "Internal database error", "description"));
        }
        odb::query<Thread> q = odb::query<Thread>::board == name() && odb::query<Thread>::number == threadNumber;
        q = q && odb::query<Thread>::archived == false;
        Database::Result<Thread> thread = Database::queryOne<Thread, Thread>(q);
        QByteArray hashpass = Tools::hashpass(app.request());
        bool modOnBoard = Database::moderOnBoard(app.request(), name());
        if (thread.error) {
            return Controller::renderError(app, tq.translate("AbstractBoard", "Internal error", "error"),
                                           tq.translate("AbstractBoard", "Internal database error", "description"));
        }
        if (!thread)
            return Controller::renderNotFound(app);
        const Thread::Posts &posts = thread->posts();
        if (posts.isEmpty()) {
            return Controller::renderError(app, tq.translate("AbstractBoard", "Internal error", "error"),
                tq.translate("AbstractBoard", "Internal database error", "description"));
        }
        c.closed = !thread->postingEnabled();
        c.fixed = thread->fixed();
        c.id = thread->id();
        c.number = thread->number();
        Post opPost = *posts.first().load();
        int lvl = Database::registeredUserLevel(app.request());
        if (opPost.premoderation() && hashpass != opPost.hashpass()
                && (!modOnBoard || Database::registeredUserLevel(opPost.hashpass()) >= lvl)) {
            return Controller::renderNotFound(app);
        }
        pageTitle = opPost.subject();
        if (pageTitle.isEmpty()) {
            pageTitle = opPost.text().replace(QRegExp("\\r?\\n+"), " ").replace(QRegExp("<[^<>]+>"), "");
            pageTitle.replace("&amp;", "&");
            pageTitle.replace("&lt;", "<");
            pageTitle.replace("&gt;", ">");
            pageTitle.replace("&nbsp;", " ");
            pageTitle.replace("&quot;", "\"");
            if (pageTitle.length() > 50)
                pageTitle = pageTitle.left(47) + "...";
        }
        postingEn = postingEn && thread->postingEnabled();
        bool ok = false;
        QString err;
        c.opPost = toController(*posts.first().load(), app.request(), &ok, &err);
        if (!ok)
            return Controller::renderError(app, tq.translate("AbstractBoard", "Internal error", "error"), err);
        foreach (int j, bRangeD(1, posts.size() - 1)) {
            Post post = *posts.at(j).load();
            if (post.premoderation() && hashpass != post.hashpass()
                    && (!modOnBoard || Database::registeredUserLevel(post.hashpass()) >= lvl)) {
                continue;
            }
            c.posts.push_back(toController(post, app.request(), &ok, &err));
            if (!ok)
                return Controller::renderError(app, tq.translate("AbstractBoard", "Internal error", "error"), err);
        }
        t.commit();
    }  catch (const odb::exception &e) {
        return Controller::renderError(app, tq.translate("AbstractBoard", "Internal error", "error"),
                                       Tools::fromStd(e.what()));
    }
    Controller::initBaseBoard(c, app.request(), this, postingEn, pageTitle, threadNumber);
    c.autoUpdateEnabled = !Tools::cookieValue(app.request(), "auto_update").compare("true", Qt::CaseInsensitive);
    c.autoUpdateText = ts.translate("AbstractBoard", "Auto update", "autoUpdateText");
    c.backText = ts.translate("AbstractBoard", "Back", "backText");
    c.bumpLimit = bumpLimit();
    c.newPostsText = ts.translate("AbstractBoard", "New posts:", "newPostsText");
    c.noNewPostsText = ts.translate("AbstractBoard", "No new posts", "noNewPostsText");
    c.postLimit = postLimit();
    c.hidden = (Tools::cookieValue(app.request(), "postHidden" + name() + QString::number(threadNumber)) == "true");
    c.updateThreadText = ts.translate("AbstractBoard", "Update thread", "updateThreadText");
    beforeRenderThread(app.request(), cc.data());
    app.render(Tools::toStd(viewName), c);
    Tools::log(app, "Handled thread");
}

bool AbstractBoard::isCaptchaValid(const cppcms::http::request &req, const Tools::PostParameters &params,
                                   QString &error) const
{
    QString captcha = params.value("g-recaptcha-response");
    TranslatorQt tq(req);
    if (captcha.isEmpty())
        return bRet(&error, tq.translate("AbstractBoard", "Captcha is empty", "error"), false);
    try {
        curlpp::Cleanup curlppCleanup;
        Q_UNUSED(curlppCleanup)
        QString url = "https://www.google.com/recaptcha/api/siteverify?secret=%1&response=%2";
        url = url.arg(SettingsLocker()->value("Site/captcha_private_key").toString()).arg(captcha);
        curlpp::Easy request;
        request.setOpt(curlpp::options::Url(Tools::toStd(url)));
        std::ostringstream os;
        os << request;
        QString result = Tools::fromStd(os.str());
        result.remove(QRegExp(".*\"success\":\\s*\"?"));
        result.remove(QRegExp("\"?\\,?\\s+.+"));
        if (result.compare("true", Qt::CaseInsensitive))
            return bRet(&error, tq.translate("AbstractBoard", "Captcha is incorrect", "error"), false);
        return true;
    } catch (curlpp::RuntimeError &e) {
        return bRet(&error, Tools::fromStd(e.what()), false);
    } catch(curlpp::LogicError &e) {
        return bRet(&error, Tools::fromStd(e.what()), false);
    }
}

bool AbstractBoard::isEnabled() const
{
    return SettingsLocker()->value("Board/" + name() + "/enabled", true).toBool();
}

bool AbstractBoard::isFileTypeSupported(const QString &mimeType) const
{
    if (mimeType.isEmpty())
        return false;
    QStringList fileTypes = supportedFileTypes().split(',');
    foreach (const QString &ft, fileTypes) {
        if (QRegExp(ft, Qt::CaseInsensitive, QRegExp::Wildcard).exactMatch(mimeType))
            return true;
    }
    return false;
}

bool AbstractBoard::isFileTypeSupported(const QByteArray &data) const
{
    return isFileTypeSupported(Tools::mimeType(data));
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

bool AbstractBoard::premoderationEnabled() const
{
    SettingsLocker s;
    return s->value("Board/" + name() + "/premoderation_enabled",
                    s->value("Board/premoderation_enabled", false)).toBool();
}

QStringList AbstractBoard::rules(const QLocale &l) const
{
    return Tools::rules("rules", l) + Tools::rules("rules/" + name(), l);
}

QStringList AbstractBoard::saveFile(const Tools::File &f, bool *ok)
{
#if defined(Q_OS_WIN)
    static const QString FfmpegDefault = "ffmpeg.exe";
#elif defined(Q_OS_UNIX)
    static const QString FfmpegDefault = "ffmpeg";
#endif
    QString storagePath = Tools::storagePath();
    if (storagePath.isEmpty())
        return bRet(ok, false, QStringList());
    QString path = storagePath + "/img/" + name();
    if (!BDirTools::mkpath(path))
        return bRet(ok, false, QStringList());
    QString dt = QString::number(QDateTime::currentDateTimeUtc().toMSecsSinceEpoch());
    QString suffix = QFileInfo(f.fileName).suffix();
    QString sfn = path + "/" + dt + "." + suffix;
    FileTransaction t;
    t.addFile(sfn);
    if (!BDirTools::writeFile(sfn, f.data))
        return bRet(ok, false, QStringList());
    QVariantMap m;
    QImage img;
    if (!suffix.compare("webm", Qt::CaseInsensitive)) {
        QString ffmpeg = SettingsLocker()->value("System/ffmpeg_command", FfmpegDefault).toString();
        QStringList args = QStringList() << "-i" << QDir::toNativeSeparators(sfn) << "-vframes" << "1"
                                         << (dt + "s.png");
        if (!BeQt::execProcess(path, ffmpeg, args, BeQt::Second, 5 * BeQt::Second)) {
            t.addFile(path + "/" + dt + "s.png");
            if (!img.load(path + "/" + dt + "s.png"))
                return bRet(ok, false, QStringList());
            scaleThumbnail(img, m);
            if (!img.save(path + "/" + dt + "s.png", "png"))
                return bRet(ok, false, QStringList());
            m.insert("thumbFileName", dt + "s.png");
        } else {
            m.insert("thumbFileName", "webm");
        }
    } else {
        QByteArray data = f.data;
        QBuffer buff(&data);
        buff.open(QIODevice::ReadOnly);
        if (!img.load(&buff, suffix.toLower().toLatin1().data()))
            return bRet(ok, false, QStringList());
        scaleThumbnail(img, m);
        if (!suffix.compare("gif", Qt::CaseInsensitive))
            suffix = "png";
        t.addFile(path + "/" + dt + "s." + suffix);
        if (!img.save(path + "/" + dt + "s." + suffix, suffix.toLower().toLatin1().data()))
            return bRet(ok, false, QStringList());
        m.insert("thumbFileName", dt + "s." + suffix);
    }
    QString ifn = path + "/" + dt + ".ololord-file-info";
    t.addFile(ifn);
    if (!BDirTools::writeFile(ifn, BeQt::serialize(m)))
        return bRet(ok, false, QStringList());
    t.commit();
    return bRet(ok, true, QStringList() << QFileInfo(sfn).fileName());
}

bool AbstractBoard::showWhois() const
{
    return false;
}

QString AbstractBoard::supportedFileTypes() const
{
    SettingsLocker s;
    return s->value("Board/" + name() + "/supported_file_types",
                    s->value("Board/supported_file_types", defaultFileTypes)).toString();
}

bool AbstractBoard::testParams(const Tools::PostParameters &params, bool /*post*/, const QLocale &l,
                               QString *error) const
{
    TranslatorQt tq(l);
    if (params.value("email").length() > int(Tools::maxInfo(Tools::MaxEmailFieldLength, name())))
        return bRet(error, tq.translate("AbstractBoard", "E-mail is too long", "description"), false);
    else if (params.value("name").length() > int(Tools::maxInfo(Tools::MaxNameFieldLength, name())))
        return bRet(error, tq.translate("AbstractBoard", "Name is too long", "description"), false);
    else if (params.value("subject").length() > int(Tools::maxInfo(Tools::MaxSubjectFieldLength, name())))
        return bRet(error, tq.translate("AbstractBoard", "Subject is too long", "description"), false);
    else if (params.value("text").length() > int(Tools::maxInfo(Tools::MaxTextFieldLength, name())))
        return bRet(error, tq.translate("AbstractBoard", "Comment is too long", "description"), false);
    else if (params.value("password").length() > int(Tools::maxInfo(Tools::MaxPasswordFieldLength, name())))
        return bRet(error, tq.translate("AbstractBoard", "Password is too long", "description"), false);
    return bRet(error, QString(), true);
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

QString AbstractBoard::thumbFileName(const QString &fn, QString &size, int &thumbSizeX, int &thumbSizeY, int &sizeX,
                                     int &sizeY) const
{
    if (fn.isEmpty())
        return "";
    QString storagePath = Tools::storagePath();
    if (storagePath.isEmpty())
        return "";
    QFileInfo fi(storagePath + "/img/" + name() + "/" + QFileInfo(fn).fileName());
    size = QString::number(fi.size() / BeQt::Kilobyte) + "KB";
    bool ok = false;
    QByteArray ba = BDirTools::readFile(storagePath + "/img/" + name() + "/" + QFileInfo(fn).baseName()
                                        + ".ololord-file-info", -1, &ok);
    if (!ok)
        return "";
    QVariantMap m = BeQt::deserialize(ba).toMap();
    sizeX = m.value("width").toInt();
    sizeY = m.value("height").toInt();
    thumbSizeX = m.value("thumbWidth").toInt();
    thumbSizeY = m.value("thumbHeight").toInt();
    size += ", " + QString::number(sizeX) + "x" + QString::number(sizeY);
    return m.value("thumbFileName").toString();
}

Content::Post AbstractBoard::toController(const Post &post, const cppcms::http::request &req, bool *ok,
                                          QString *error) const
{
    TranslatorQt tq(req);
    QString storagePath = Tools::storagePath();
    if (storagePath.isEmpty()) {
        return bRet(ok, false, error, tq.translate("AbstractBoard", "Internal file system error", "error"),
                    Content::Post());
    }
    Content::Post *p = Cache::post(name(), post.number());
    bool inCache = p;
    if (!p) {
        p = new Content::Post;
        p->bannedFor = post.bannedFor();
        p->email = Tools::toStd(post.email());
        p->number = post.number();
        p->rawSubject = Tools::toStd(post.subject());
        p->subject = p->rawSubject;
        p->subjectIsRaw = false;
        p->rawName = Tools::toStd(post.name());
        p->premoderation = post.premoderation();
        foreach (const QString &fn, post.files()) {
            QFileInfo fi(fn);
            Content::File f;
            f.type = fn.endsWith("webm", Qt::CaseInsensitive) ? "webm" : "image";
            f.sourceName = Tools::toStd(fi.fileName());
            QString sz;
            f.thumbName = Tools::toStd(thumbFileName(fi.fileName(), sz, f.thumbSizeX, f.thumbSizeY, f.sizeX, f.sizeY));
            f.size = Tools::toStd(sz);
            p->files.push_back(f);
        }
        quint64 threadNumber = 0;
        try {
            Transaction t;
            if (!t) {
                return bRet(ok, false, error, tq.translate("AbstractBoard", "Internal database error", "error"),
                            Content::Post());
            }
            QSharedPointer<Thread> thread = post.thread().load();
            threadNumber = thread->number();
            bool op = (post.number() == threadNumber);
            p->fixed = op && thread->fixed();
            p->closed = op && !thread->postingEnabled();
            t.commit();
        } catch (const odb::exception &e) {
            return bRet(ok, false, error, Tools::fromStd(e.what()), Content::Post());
        }
        p->text = Tools::toStd(post.text());
        p->rawPostText = Tools::toStd(post.rawText());
        p->threadNumber = threadNumber;
        p->showRegistered = false;
        p->showTripcode = post.showTripcode();
        if (showWhois()) {
            QString countryCode = Tools::countryCode(post.posterIp());
            p->flagName = Tools::toStd(Tools::flagName(countryCode));
            if (!p->flagName.empty()) {
                p->countryName = Tools::toStd(Tools::countryName(countryCode));
                if (SettingsLocker()->value("Board/guess_city_name", true).toBool())
                    p->cityName = Tools::toStd(Tools::cityName(post.posterIp()));
            } else {
                p->flagName = "default.png";
                p->countryName = "Unknown country";
            }
        }
        QList<quint64> list = post.referencedBy().toList();
        qSort(list);
        foreach (quint64 pn, list)
            p->referencedBy.push_back(pn);
    }
    Content::Post pp = *p;
    if (!inCache && !Cache::cachePost(name(), post.number(), p))
        delete p;
    TranslatorStd ts(req);
    QLocale l = tq.locale();
    for (std::list<Content::File>::iterator i = pp.files.begin(); i != pp.files.end(); ++i)
        i->size = Tools::toStd(Tools::fromStd(i->size).replace("KB", tq.translate("AbstractBoard", "KB", "fileSize")));
    if (showWhois() && "Unknown country" == pp.countryName)
        pp.countryName = ts.translate("AbstractBoard", "Unknown country", "countryName");
    int regLvl = Database::registeredUserLevel(req);
    if (regLvl >= RegisteredUser::ModerLevel)
        pp.ip = Tools::toStd(post.posterIp());
    pp.dateTime = Tools::toStd(l.toString(Tools::dateTime(post.dateTime(), req), "dd/MM/yyyy ddd hh:mm:ss"));
    pp.hidden = (Tools::cookieValue(req, "postHidden" + name() + QString::number(post.number())) == "true");
    pp.name = Tools::toStd(Controller::toHtml(post.name()));
    if (pp.name.empty())
        pp.name = "<span class=\"userName\">" + Tools::toStd(Controller::toHtml(defaultUserName(l))) + "</span>";
    else
        pp.name = "<span class=\"userName\">" + pp.name + "</span>";
    pp.nameRaw = Tools::toStd(post.name());
    if (pp.nameRaw.empty())
        pp.nameRaw = Tools::toStd(defaultUserName(l));
    QByteArray hashpass = post.hashpass();
    if (!hashpass.isEmpty()) {
        int lvl = Database::registeredUserLevel(hashpass);
        QString name;
        pp.showRegistered = lvl >= RegisteredUser::UserLevel;
        if (!post.name().isEmpty()) {
            if (lvl >= RegisteredUser::AdminLevel)
                name = post.name();
            else if (lvl >= RegisteredUser::ModerLevel)
                name = "<span class=\"moderName\">" + Controller::toHtml(post.name()) + "</span>";
            else if (lvl >= RegisteredUser::UserLevel)
                name = "<span class=\"userName\">" + Controller::toHtml(post.name()) + "</span>";
        }
        pp.name = Tools::toStd(name);
        if (pp.name.empty())
            pp.name = "<span class=\"userName\">" + Tools::toStd(Controller::toHtml(defaultUserName(l))) + "</span>";
        QString s;
        hashpass += SettingsLocker()->value("Site/tripcode_salt").toString().toUtf8();
        QByteArray tripcode = QCryptographicHash::hash(hashpass, QCryptographicHash::Md5);
        foreach (int i, bRangeD(0, tripcode.size() - 1)) {
            QChar c(tripcode.at(i));
            if (c.isLetterOrNumber() || c.isPunct())
                s += c;
            else
                s += QString::number(uchar(tripcode.at(i)), 16);
        }
        pp.tripcode = Tools::toStd(s);
    }
    return bRet(ok, true, error, QString(), pp);
}

void AbstractBoard::beforeRenderBoard(const cppcms::http::request &/*req*/, Content::Board */*c*/)
{
    //
}

void AbstractBoard::beforeRenderThread(const cppcms::http::request &/*req*/, Content::Thread */*c*/)
{
    //
}

void AbstractBoard::beforeStoring(Tools::PostParameters &/*params*/, bool /*post*/)
{
    //
}

Content::Board *AbstractBoard::createBoardController(const cppcms::http::request &/*req*/, QString &/*viewName*/)
{
    return new Content::Board;
}

Content::Thread *AbstractBoard::createThreadController(const cppcms::http::request &/*req*/, QString &/*viewName*/)
{
    return new Content::Thread;
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
    b = new echoBoard;
    boards.insert(b->name(), b);
    b = new hBoard;
    boards.insert(b->name(), b);
    b = new intBoard;
    boards.insert(b->name(), b);
    b = new mlpBoard;
    boards.insert(b->name(), b);
    b = new prBoard;
    boards.insert(b->name(), b);
    b = new rfBoard;
    boards.insert(b->name(), b);
    b = new socBoard;
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
        nnn->setDescription(BTranslation::translate("AbstractBoard",
                                                    "Determines if captcha is enabled on this board.\n"
                                                    "The default is true."));
        nnn = new BSettingsNode(QVariant::UInt, "threads_per_page", nn);
        nnn->setDescription(BTranslation::translate("AbstractBoard", "Number of threads per one page on this board.\n"
                                                    "The default is 20."));
        nnn = new BSettingsNode(QVariant::Bool, "posting_enabled", nn);
        nnn->setDescription(BTranslation::translate("AbstractBoard",
                                                    "Determines if posting is enabled on this board.\n"
                                                    "The default is true."));
        nnn = new BSettingsNode(QVariant::Bool, "premoderation_enabled", nn);
        nnn->setDescription(BTranslation::translate("initSettings",
                                                    "Determines if pre-moderation is enabled on this board.\n"
                                                    "The default is false."));
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
        nnn = new BSettingsNode(QVariant::UInt, "max_last_posts", nn);
        nnn->setDescription(BTranslation::translate("AbstractBoard", "Maximum last posts displayed for each thread at "
                                                    "this board.\nThe default is 3."));
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
        nnn = new BSettingsNode(QVariant::UInt, "captcha_quota", nn);
        nnn->setDescription(BTranslation::translate("AbstractBoard", "Maximum count of extra posts a user may make "
                                                    "before solving captcha again on this board.\n"
                                                    "The default is 0 (solve captcha every time)."));
        nnn = new BSettingsNode(QVariant::String, "supported_file_types", nn);
        BTranslation t = BTranslation::translate("AbstractBoard", "MIME types of files allowed for attaching on "
                                                 "this board.\n"
                                                 "Must be separated by commas. Wildcard matching is used.\n"
                                                 "The default is %1.");
        t.setArgument(defaultFileTypes);
        nnn->setDescription(t);
    }
    if (!reinit)
        qAddPostRoutine(&cleanupBoards);
    boardsInitialized = true;
}
