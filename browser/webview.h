#ifndef MYWEBVIEW_H
#define MYWEBVIEW_H

QT_BEGIN_NAMESPACE
class QAuthenticator;
class QMouseEvent;
class QNetworkProxy;
class QNetworkReply;
class QSslError;
QT_END_NAMESPACE


class WebPage;
class WebView : public QWebEngineView {
    Q_OBJECT

public:
    WebView(QWidget *parent = 0);
    WebPage *webPage() const { return m_page; }
    void setPage(WebPage *page);

    void loadUrl(const QUrl &url);
    QUrl url() const;

    QString lastStatusBarText() const;
    inline int progress() const { return m_progress; }

    void toHtml(std::function<void(const QString&)>);

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void contextMenuEvent(QContextMenuEvent *event);
    void wheelEvent(QWheelEvent *event);

private slots:
    void setProgress(int progress);
    void loadFinished(bool success);
    void setStatusBarText(const QString &string);
    void onFeaturePermissionRequested(const QUrl &securityOrigin, QWebEnginePage::Feature);
    void onIconChanged(const QIcon &icon);

private:
    QString m_statusBarText;
    QUrl m_initialUrl;
    int m_progress;
    WebPage *m_page;
};

#endif
