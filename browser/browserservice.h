#ifndef BROWSER_CONTEXT_H
#define BROWSER_CONTEXT_H

#include <QtWidgets/QApplication>

#include <QtCore/QUrl>
#include <QtCore/QPointer>

#include <QtGui/QIcon>

#include <QtNetwork/QAuthenticator>

QT_BEGIN_NAMESPACE
class QLocalServer;
class QNetworkAccessManager;
class QWebEngineProfile;
QT_END_NAMESPACE

class BookmarksManager;
class BrowserWnd;
class CookieJar;
class DownloadManager;
class HistoryManager;

#include "kutil/singleton.h"

class BrowserService : public SingletonWithBase<BrowserService, QObject>
{
    Q_OBJECT

        friend class SingletonWithBase<BrowserService, QObject>;

    BrowserService();
    ~BrowserService();

public:
    void loadSettings();

    BrowserWnd *browser();
    QList<BrowserWnd*> browsers();
    QIcon icon(const QUrl &url) const;
    QIcon defaultIcon() const;

    void saveSession();
    bool canRestoreSession() const;
    bool privateBrowsing() const { return m_privateBrowsing; }

    static HistoryManager *historyMan();
    static CookieJar *cookieJar();
    static DownloadManager *downloadMan();
    static QNetworkAccessManager *networkAccessMan();
    static BookmarksManager *bookmarksMan();

#if defined(Q_OS_OSX)
    bool event(QEvent *event);
#endif

public slots:
    BrowserWnd *newBroswer();
    void restoreLastSession();
    void lastWindowClosed();
    void setPrivateBrowsing(bool);

signals:
    void privateBrowsingChanged(bool);

private slots:
    void postLaunch();
    void openUrl(const QUrl &url);

private:
    void clean();
    void installTranslator(const QString &name);

    HistoryManager *s_historyManager = nullptr;
    DownloadManager *s_downloadManager = nullptr;
    QNetworkAccessManager *s_networkAccessManager = nullptr;
    BookmarksManager *s_bookmarksManager = nullptr;

    QList<QPointer<BrowserWnd> > m_mainWindows;
    QByteArray m_lastSession;
    QWebEngineProfile *m_privateProfile;
    bool m_privateBrowsing = false;
    mutable QIcon m_defaultIcon;

    QAuthenticator m_lastAuthenticator;
    QAuthenticator m_lastProxyAuthenticator;
};

#endif // BROWSER_CONTEXT_H
