#include "stdafx.h"
#include "popupwindow.h"
#include "webview.h"
#include "webpage.h"

PopupWindow::PopupWindow(QWebEngineProfile *profile)
    : m_addressBar(new QLineEdit(this))
    , m_view(new WebView(this))
{
    m_view->setPage(new WebPage(profile, m_view));
    QVBoxLayout *layout = new QVBoxLayout;
    layout->setMargin(0);
    setLayout(layout);
    layout->addWidget(m_addressBar);
    layout->addWidget(m_view);
    m_view->setFocus();

    connect(m_view, &WebView::titleChanged, this, &QWidget::setWindowTitle);
    connect(m_view, &WebView::urlChanged, this, &PopupWindow::setUrl);
    connect(page(), &WebPage::geometryChangeRequested, this, &PopupWindow::adjustGeometry);
    connect(page(), &WebPage::windowCloseRequested, this, &QWidget::close);
}

QWebEnginePage* PopupWindow::page() const
{
    return m_view->page();
}

void PopupWindow::setUrl(const QUrl &url)
{
    m_addressBar->setText(url.toString());
}

void PopupWindow::adjustGeometry(const QRect &newGeometry)
{
    const int x1 = frameGeometry().left() - geometry().left();
    const int y1 = frameGeometry().top() - geometry().top();
    const int x2 = frameGeometry().right() - geometry().right();
    const int y2 = frameGeometry().bottom() - geometry().bottom();

    setGeometry(newGeometry.adjusted(x1, y1 - m_addressBar->height(), x2, y2));
}

