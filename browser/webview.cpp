#include "stdafx.h"

#include "webview.h"
#include "browserapplication.h"
#include "cookiejar.h"
#include "downloadmanager.h"
#include "featurepermissionbar.h"
#include "ui_passworddialog.h"
#include "ui_proxy.h"
#include "tabwidget.h"
#include "webpage.h"

WebView::WebView(QWidget* parent)
    : QWebEngineView(parent)
    , m_progress(0)
    , m_page(0)
{
    connect(this, SIGNAL(loadProgress(int)),
            this, SLOT(setProgress(int)));
    connect(this, SIGNAL(loadFinished(bool)),
            this, SLOT(loadFinished(bool)));
    connect(this, &QWebEngineView::renderProcessTerminated,
            [=](QWebEnginePage::RenderProcessTerminationStatus termStatus, int statusCode) {
        const char *status = "";
        switch (termStatus) {
        case QWebEnginePage::NormalTerminationStatus:
            status = "(normal exit)";
            break;
        case QWebEnginePage::AbnormalTerminationStatus:
            status = "(abnormal exit)"; 
            break;
        case QWebEnginePage::CrashedTerminationStatus:
            status = "(crashed)";
            break;
        case QWebEnginePage::KilledTerminationStatus:
            status = "(killed)";
            break;
        }

        qInfo() << "Render process exited with code" << statusCode << status;
        QTimer::singleShot(0, [this] { reload(); });
    });
}

void WebView::setPage(WebPage *_page)
{
    if (m_page)
        m_page->deleteLater();
    m_page = _page;
    QWebEngineView::setPage(_page);
#if defined(QWEBENGINEPAGE_STATUSBARMESSAGE)
    connect(page(), SIGNAL(statusBarMessage(QString)),
            SLOT(setStatusBarText(QString)));
#endif
    disconnect(page(), &QWebEnginePage::iconChanged, this, &WebView::iconChanged);
    connect(page(), SIGNAL(iconChanged(QIcon)),
            this, SLOT(onIconChanged(QIcon)));
    connect(page(), &WebPage::featurePermissionRequested, this, &WebView::onFeaturePermissionRequested);
#if defined(QWEBENGINEPAGE_UNSUPPORTEDCONTENT)
    page()->setForwardUnsupportedContent(true);
#endif
}

void WebView::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu *menu;
    if (page()->contextMenuData().linkUrl().isValid()) {
        menu = new QMenu(this);
        menu->addAction(page()->action(QWebEnginePage::OpenLinkInThisWindow));
        menu->addAction(page()->action(QWebEnginePage::OpenLinkInNewWindow));
        menu->addAction(page()->action(QWebEnginePage::OpenLinkInNewTab));
        menu->addAction(page()->action(QWebEnginePage::OpenLinkInNewBackgroundTab));
        menu->addSeparator();
        menu->addAction(page()->action(QWebEnginePage::DownloadLinkToDisk));
        menu->addAction(page()->action(QWebEnginePage::CopyLinkToClipboard));
    } else {
        menu = page()->createStandardContextMenu();
    }
    if (page()->contextMenuData().selectedText().isEmpty())
        menu->addAction(page()->action(QWebEnginePage::SavePage));
    connect(menu, &QMenu::aboutToHide, menu, &QObject::deleteLater);
    menu->popup(event->globalPos());
}

void WebView::wheelEvent(QWheelEvent *event)
{
#if defined(QWEBENGINEPAGE_SETTEXTSIZEMULTIPLIER)
    if (QApplication::keyboardModifiers() & Qt::ControlModifier) {
        int numDegrees = event->delta() / 8;
        int numSteps = numDegrees / 15;
        setTextSizeMultiplier(textSizeMultiplier() + numSteps * 0.1);
        event->accept();
        return;
    }
#endif
    QWebEngineView::wheelEvent(event);
}

void WebView::onFeaturePermissionRequested(const QUrl &securityOrigin, QWebEnginePage::Feature feature)
{
    FeaturePermissionBar *permissionBar = new FeaturePermissionBar(this);
    connect(permissionBar, &FeaturePermissionBar::featurePermissionProvided, page(), &QWebEnginePage::setFeaturePermission);

    // Discard the bar on new loads (if we navigate away or reload).
    connect(page(), &QWebEnginePage::loadStarted, permissionBar, &QObject::deleteLater);

    permissionBar->requestPermission(securityOrigin, feature);
}

void WebView::setProgress(int progress)
{
    m_progress = progress;
}

void WebView::loadFinished(bool success)
{
    if (success && 100 != m_progress) {
        qWarning() << "Received finished signal while progress is still:" << progress()
                   << "Url:" << url();
    }
    m_progress = 0;
}

void WebView::loadUrl(const QUrl &url)
{
    m_initialUrl = url;
    load(url);
}

QString WebView::lastStatusBarText() const
{
    return m_statusBarText;
}

void WebView::toHtml(std::function<void(const QString&)> resultCB)
{
    page()->toHtml(resultCB);
}

QUrl WebView::url() const
{
    QUrl url = QWebEngineView::url();
    if (!url.isEmpty())
        return url;

    return m_initialUrl;
}

void WebView::onIconChanged(const QIcon &icon)
{
    if (icon.isNull())
        emit iconChanged(BrowserAppCtx::instance()->defaultIcon());
    else
        emit iconChanged(icon);
}

void WebView::mousePressEvent(QMouseEvent *event)
{
    m_page->m_pressedButtons = event->buttons();
    m_page->m_keyboardModifiers = event->modifiers();
    QWebEngineView::mousePressEvent(event);
}

void WebView::mouseReleaseEvent(QMouseEvent *event)
{
    QWebEngineView::mouseReleaseEvent(event);
    if (!event->isAccepted() && (m_page->m_pressedButtons & Qt::MidButton)) {
        QUrl url(QApplication::clipboard()->text(QClipboard::Selection));
        if (!url.isEmpty() && url.isValid() && !url.scheme().isEmpty()) {
            setUrl(url);
        }
    }
}

void WebView::setStatusBarText(const QString &string)
{
    m_statusBarText = string;
}
