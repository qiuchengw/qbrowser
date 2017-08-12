#include "stdafx.h"
#include "browserservice.h"
#include "bookmarks.h"
#include "browserwnd.h"
#include "cookiejar.h"
#include "downloadmanager.h"
#include "history.h"
#include "tabwidget.h"
#include "webview.h"

static void setUserStyleSheet(QWebEngineProfile *profile, const QString &styleSheet, BrowserWnd *wnd = 0)
{
    Q_ASSERT(profile);
    QString scriptName(QStringLiteral("userStyleSheet"));
    QWebEngineScript script;
    QList<QWebEngineScript> styleSheets = profile->scripts()->findScripts(scriptName);
    if (!styleSheets.isEmpty())
        script = styleSheets.first();
    Q_FOREACH(const QWebEngineScript &s, styleSheets)
        profile->scripts()->remove(s);

    if (script.isNull()) {
        script.setName(scriptName);
        script.setInjectionPoint(QWebEngineScript::DocumentReady);
        script.setRunsOnSubFrames(true);
        script.setWorldId(QWebEngineScript::ApplicationWorld);
    }
    QString source = QString::fromLatin1("(function() {"\
        "var css = document.getElementById(\"_qt_testBrowser_userStyleSheet\");"\
        "if (css == undefined) {"\
        "    css = document.createElement(\"style\");"\
        "    css.type = \"text/css\";"\
        "    css.id = \"_qt_testBrowser_userStyleSheet\";"\
        "    document.head.appendChild(css);"\
        "}"\
        "css.innerText = \"%1\";"\
        "})()").arg(styleSheet);
    script.setSourceCode(source);
    profile->scripts()->insert(script);
    // run the script on the already loaded views
    // this has to be deferred as it could mess with the storage initialization on startup
    if (wnd)
        QMetaObject::invokeMethod(wnd, "runScriptOnOpenViews", Qt::QueuedConnection, Q_ARG(QString, source));
}

BrowserService::BrowserService()
    : SingletonWithBase<BrowserService, QObject>(nullptr)
    , m_privateProfile(0)
    , m_privateBrowsing(false)
{
    QString localSysName = QLocale::system().name();
    installTranslator(QLatin1String("qt_") + localSysName);

    QSettings settings;
    settings.beginGroup(QLatin1String("sessions"));
    m_lastSession = settings.value(QLatin1String("lastSession")).toByteArray();
    settings.endGroup();
}

BrowserService::~BrowserService()
{
    delete s_downloadManager;
    for (int i = 0; i < m_mainWindows.size(); ++i) {
        BrowserWnd *window = m_mainWindows.at(i);
        delete window;
    }
    delete s_networkAccessManager;
    delete s_bookmarksManager;
}

void BrowserService::lastWindowClosed()
{
#if defined(Q_OS_OSX)
    clean();
    BrowserWnd *mw = new BrowserWnd;
    mw->slotHome();
    m_mainWindows.prepend(mw);
#endif
}

/*!
    Any actions that can be delayed until the window is visible
 */
void BrowserService::postLaunch()
{
    QString directory = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
    if (directory.isEmpty())
        directory = QDir::homePath() + QLatin1String("/.") + QCoreApplication::applicationName();
#if defined(QWEBENGINESETTINGS_PATHS)
    QWebEngineSettings::setIconDatabasePath(directory);
    QWebEngineSettings::setOfflineStoragePath(directory);
#endif

    loadSettings();

    BrowserService::historyMan();
}

void BrowserService::loadSettings()
{
    QSettings settings;
    settings.beginGroup(QLatin1String("websettings"));

    QWebEngineSettings *defaultSettings = QWebEngineSettings::globalSettings();
    QWebEngineProfile *defaultProfile = QWebEngineProfile::defaultProfile();

    QString standardFontFamily = defaultSettings->fontFamily(QWebEngineSettings::StandardFont);
    int standardFontSize = defaultSettings->fontSize(QWebEngineSettings::DefaultFontSize);
    QFont standardFont = QFont(standardFontFamily, standardFontSize);
    standardFont = qvariant_cast<QFont>(settings.value(QLatin1String("standardFont"), standardFont));
    defaultSettings->setFontFamily(QWebEngineSettings::StandardFont, standardFont.family());
    defaultSettings->setFontSize(QWebEngineSettings::DefaultFontSize, standardFont.pointSize());

    QString fixedFontFamily = defaultSettings->fontFamily(QWebEngineSettings::FixedFont);
    int fixedFontSize = defaultSettings->fontSize(QWebEngineSettings::DefaultFixedFontSize);
    QFont fixedFont = QFont(fixedFontFamily, fixedFontSize);
    fixedFont = qvariant_cast<QFont>(settings.value(QLatin1String("fixedFont"), fixedFont));
    defaultSettings->setFontFamily(QWebEngineSettings::FixedFont, fixedFont.family());
    defaultSettings->setFontSize(QWebEngineSettings::DefaultFixedFontSize, fixedFont.pointSize());

    defaultSettings->setAttribute(QWebEngineSettings::JavascriptEnabled, settings.value(QLatin1String("enableJavascript"), true).toBool());
    defaultSettings->setAttribute(QWebEngineSettings::ScrollAnimatorEnabled, settings.value(QLatin1String("enableScrollAnimator"), true).toBool());

    defaultSettings->setAttribute(QWebEngineSettings::PluginsEnabled, settings.value(QLatin1String("enablePlugins"), true).toBool());

    defaultSettings->setAttribute(QWebEngineSettings::FullScreenSupportEnabled, true);

    QString css = settings.value(QLatin1String("userStyleSheet")).toString();
    setUserStyleSheet(defaultProfile, css, browser());

    defaultProfile->setHttpUserAgent(settings.value(QLatin1String("httpUserAgent")).toString());
    defaultProfile->setHttpAcceptLanguage(settings.value(QLatin1String("httpAcceptLanguage")).toString());

    switch (settings.value(QLatin1String("faviconDownloadMode"), 1).toInt()) {
    case 0:
        defaultSettings->setAttribute(QWebEngineSettings::AutoLoadIconsForPage, false);
        break;
    case 1:
        defaultSettings->setAttribute(QWebEngineSettings::AutoLoadIconsForPage, true);
        defaultSettings->setAttribute(QWebEngineSettings::TouchIconsEnabled, false);
        break;
    case 2:
        defaultSettings->setAttribute(QWebEngineSettings::AutoLoadIconsForPage, true);
        defaultSettings->setAttribute(QWebEngineSettings::TouchIconsEnabled, true);
        break;
    }

    settings.endGroup();
    settings.beginGroup(QLatin1String("cookies"));

    QWebEngineProfile::PersistentCookiesPolicy persistentCookiesPolicy = QWebEngineProfile::PersistentCookiesPolicy(settings.value(QLatin1String("persistentCookiesPolicy")).toInt());
    defaultProfile->setPersistentCookiesPolicy(persistentCookiesPolicy);
    QString pdataPath = settings.value(QLatin1String("persistentDataPath")).toString();
    defaultProfile->setPersistentStoragePath(pdataPath);

    settings.endGroup();

    settings.beginGroup(QLatin1String("proxy"));
    QNetworkProxy proxy;
    if (settings.value(QLatin1String("enabled"), false).toBool()) {
        if (settings.value(QLatin1String("type"), 0).toInt() == 0)
            proxy = QNetworkProxy::Socks5Proxy;
        else
            proxy = QNetworkProxy::HttpProxy;
        proxy.setHostName(settings.value(QLatin1String("hostName")).toString());
        proxy.setPort(settings.value(QLatin1String("port"), 1080).toInt());
        proxy.setUser(settings.value(QLatin1String("userName")).toString());
        proxy.setPassword(settings.value(QLatin1String("password")).toString());
        QNetworkProxy::setApplicationProxy(proxy);
    }
    settings.endGroup();
}

QList<BrowserWnd*> BrowserService::browsers()
{
    clean();
    QList<BrowserWnd*> list;
    for (int i = 0; i < m_mainWindows.count(); ++i)
        list.append(m_mainWindows.at(i));
    return list;
}

void BrowserService::clean()
{
    // cleanup any deleted main windows first
    for (int i = m_mainWindows.count() - 1; i >= 0; --i)
        if (m_mainWindows.at(i).isNull())
            m_mainWindows.removeAt(i);
}

void BrowserService::saveSession()
{
    if (m_privateBrowsing)
        return;

    clean();

    QSettings settings;
    settings.beginGroup(QLatin1String("sessions"));

    QByteArray data;
    QBuffer buffer(&data);
    QDataStream stream(&buffer);
    buffer.open(QIODevice::ReadWrite);

    stream << m_mainWindows.count();
    for (int i = 0; i < m_mainWindows.count(); ++i)
        stream << m_mainWindows.at(i)->saveState();
    settings.setValue(QLatin1String("lastSession"), data);
    settings.endGroup();
}

bool BrowserService::canRestoreSession() const
{
    return !m_lastSession.isEmpty();
}

void BrowserService::restoreLastSession()
{
    QList<QByteArray> windows;
    QBuffer buffer(&m_lastSession);
    QDataStream stream(&buffer);
    buffer.open(QIODevice::ReadOnly);
    int windowCount;
    stream >> windowCount;
    for (int i = 0; i < windowCount; ++i) {
        QByteArray windowState;
        stream >> windowState;
        windows.append(windowState);
    }
    for (int i = 0; i < windows.count(); ++i) {
        BrowserWnd *newWindow = 0;
        if (m_mainWindows.count() == 1
            && browser()->tabWidget()->count() == 1
            && browser()->currentTab()->url() == QUrl()) {
            newWindow = browser();
        }
        else {
            newWindow = newBroswer();
        }
        newWindow->restoreState(windows.at(i));
    }
}

void BrowserService::installTranslator(const QString &name)
{
    QTranslator *translator = new QTranslator(this);
    translator->load(name, QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    QApplication::installTranslator(translator);
}

#if defined(Q_OS_OSX)
bool BrowserService::event(QEvent* event)
{
    switch (event->type()) {
    case QEvent::ApplicationActivate: {
        clean();
        if (!m_mainWindows.isEmpty()) {
            BrowserWnd *mw = browser();
            if (mw && !mw->isMinimized()) {
                browser()->show();
            }
            return true;
        }
    }
    case QEvent::FileOpen:
        if (!m_mainWindows.isEmpty()) {
            browser()->loadPage(static_cast<QFileOpenEvent *>(event)->file());
            return true;
        }
    default:
        break;
    }
    return QApplication::event(event);
}
#endif

void BrowserService::openUrl(const QUrl &url)
{
    browser()->loadPage(url.toString());
}

BrowserWnd *BrowserService::newBroswer()
{
    BrowserWnd *browser = new BrowserWnd();
    m_mainWindows.prepend(browser);
    browser->show();
    return browser;
}

BrowserWnd *BrowserService::browser()
{
    clean();
    if (m_mainWindows.isEmpty())
        newBroswer();

    return m_mainWindows[0];
}

CookieJar *BrowserService::cookieJar()
{
#if defined(QWEBENGINEPAGE_SETNETWORKACCESSMANAGER)
    return (CookieJar*)networkAccessMan()->cookieJar();
#else
    return 0;
#endif
}

DownloadManager *BrowserService::downloadMan()
{
    BrowserService* inst = BrowserService::instance();
    if (!inst->s_downloadManager) {
        inst->s_downloadManager = new DownloadManager();
    }
    return inst->s_downloadManager;
}

QNetworkAccessManager *BrowserService::networkAccessMan()
{
    BrowserService* inst = BrowserService::instance();
    if (!inst->s_networkAccessManager) {
        inst->s_networkAccessManager = new QNetworkAccessManager();
    }
    return inst->s_networkAccessManager;
}

HistoryManager *BrowserService::historyMan()
{
    BrowserService* inst = BrowserService::instance();
    if (!inst->s_historyManager)
        inst->s_historyManager = new HistoryManager();
    return inst->s_historyManager;
}

BookmarksManager *BrowserService::bookmarksMan()
{
    BrowserService* inst = BrowserService::instance();
    if (!inst->s_bookmarksManager) {
        inst->s_bookmarksManager = new BookmarksManager;
    }
    return inst->s_bookmarksManager;
}

QIcon BrowserService::icon(const QUrl &url) const
{
#if defined(QTWEBENGINE_ICONDATABASE)
    QIcon icon = QWebEngineSettings::iconForUrl(url);
    if (!icon.isNull())
        return icon.pixmap(16, 16);
#else
    Q_UNUSED(url);
#endif
    return defaultIcon();
}

QIcon BrowserService::defaultIcon() const
{
    if (m_defaultIcon.isNull())
        m_defaultIcon = QIcon(QLatin1String(":defaulticon.png"));
    return m_defaultIcon;
}

void BrowserService::setPrivateBrowsing(bool privateBrowsing)
{
    if (m_privateBrowsing == privateBrowsing)
        return;
    m_privateBrowsing = privateBrowsing;
    if (privateBrowsing) {
        if (!m_privateProfile)
            m_privateProfile = new QWebEngineProfile(this);
        Q_FOREACH(BrowserWnd* window, browsers()) {
            window->tabWidget()->setProfile(m_privateProfile);
        }
    }
    else {
        Q_FOREACH(BrowserWnd* window, browsers()) {
            window->tabWidget()->setProfile(QWebEngineProfile::defaultProfile());
            window->m_lastSearch = QString();
            window->tabWidget()->clear();
        }
    }
    emit privateBrowsingChanged(privateBrowsing);
}
