#ifndef BROWSERMAINWINDOW_H
#define BROWSERMAINWINDOW_H

#include <QtWidgets/QMainWindow>
#include <QtGui/QIcon>
#include <QtCore/QUrl>
#include "browser.h"

QT_BEGIN_NAMESPACE
#ifndef QT_NO_PRINTER
class QPrinter;
#endif
class QWebEnginePage;
QT_END_NAMESPACE

class AutoSaver;
class BookmarksToolBar;
class ChaseWidget;
class TabWidget;
class ToolbarSearch;
class WebView;
class WorkerStack;

class BrowserFunctionPanel;
class BrowserWnd : public QMainWindow {
    Q_OBJECT

public:
    BrowserWnd(QWidget *parent = 0, Qt::WindowFlags flags = 0);
    ~BrowserWnd();
    QSize sizeHint() const;

    static const char *DEFAULT_HOME;

    void addFunctionPanel(BrowserFunctionPanel* panel);

    void setDestoryNotify(BrowserDestoryCallback d) {
        m_onDestroy = d;
    }

    // 请求创建browser的
    void setCreateBrowserDelegate(CreateBrowserDelegater d) {
        m_createBrowser = d;
    }

public:
    TabWidget *tabWidget() const;
    WebView *currentTab() const;
    WebView *viewOfTab(int idx) const;
    std::vector<WebView *>allViews(int idx) const;
    WebView* newTab(bool set_current = true);

    QByteArray saveState(bool withTabs = true) const;
    bool restoreState(const QByteArray &state);
    Q_INVOKABLE void runScriptOnOpenViews(const QString &);

public slots:
    void loadPage(const QString &url);
    WebView* loadUrl(const QUrl &url, bool new_tab, bool background);
    void slotHome();

protected:
    void closeEvent(QCloseEvent *event);

private slots:
    void save();

    void slotLoadProgress(int);
    void slotUpdateStatusbar(const QString &string);
    void slotUpdateWindowTitle(const QString &title = QString());

    void slotPreferences();

    void slotFileNew();
    void slotFileOpen();
    void slotFilePrintPreview();
    void slotFilePrint();
    void slotFilePrintToPDF();
    void slotPrivateBrowsing();
    void slotFileSaveAs();
    void slotEditFind();
    void slotEditFindNext();
    void slotEditFindPrevious();
    void slotShowBookmarksDialog();
    void slotAddBookmark();
    void slotViewZoomIn();
    void slotViewZoomOut();
    void slotViewResetZoom();
    void slotViewToolbar();
    void slotViewBookmarksBar();
    void slotViewStatusbar();
    void slotViewFullScreen(bool enable);

    void slotWebSearch();
    void slotToggleInspector(bool enable);
    void slotAboutApplication();
    void slotDownloadManager();
    void slotSelectLineEdit();

    void slotAboutToShowBackMenu();
    void slotAboutToShowForwardMenu();
    void slotAboutToShowWindowMenu();
    void slotOpenActionUrl(QAction *action);
    void slotShowWindow();
    void slotSwapFocus();
    void slotHandlePdfPrinted(const QByteArray&);

#ifndef QT_NO_PRINTER
    void slotHandlePagePrinted(bool result);
    void printRequested(QWebEnginePage *page);
#endif
    void geometryChangeRequested(const QRect &geometry);
    void updateToolbarActionText(bool visible);
    void updateBookmarksToolbarActionText(bool visible);

private:
    void loadDefaultState();
    void setupMenu();
    void setupToolBar();
    void updateStatusbarActionText(bool visible);
    void handleFindTextResult(bool found);

private:
    QToolBar *m_navigationBar;
    ToolbarSearch *m_toolbarSearch;
    BookmarksToolBar *m_bookmarksToolbar;
    ChaseWidget *m_chaseWidget;
    TabWidget *m_tabWidget;
    WorkerStack* m_workerstack;
    AutoSaver *m_autoSaver;

    QAction *m_historyBack;
    QMenu *m_historyBackMenu;
    QAction *m_historyForward;
    QMenu *m_historyForwardMenu;
    QMenu *m_windowMenu;

    QAction *m_stop;
    QAction *m_reload;
    QAction *m_stopReload;
    QAction *m_viewToolbar;
    QAction *m_viewBookmarkBar;
    QAction *m_viewStatusbar;
    QAction *m_restoreLastSession;
    QAction *m_addBookmark;

#ifndef QT_NO_PRINTER
    QPrinter *m_currentPrinter;
#endif

    QIcon m_reloadIcon;
    QIcon m_stopIcon;

    QString m_lastSearch;
    QString m_printerOutputFileName;
    friend class BrowserService;

    BrowserDestoryCallback m_onDestroy;
    CreateBrowserDelegater m_createBrowser;
};

#endif // BROWSERMAINWINDOW_H
