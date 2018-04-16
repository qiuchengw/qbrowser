#pragma once


class BrowserWnd;
class BROWSER_EXPORT WebPage : public QWebEnginePage {
    Q_OBJECT
public:
    WebPage(QWebEngineProfile *profile, QObject *parent = 0);
    BrowserWnd *mainWindow();

protected:
    QWebEnginePage *createWindow(QWebEnginePage::WebWindowType type) Q_DECL_OVERRIDE;
#if !defined(QT_NO_UITOOLS)
    QObject *createPlugin(const QString &classId, const QUrl &url, const QStringList &paramNames, const QStringList &paramValues);
#endif
    virtual bool certificateError(const QWebEngineCertificateError &error) Q_DECL_OVERRIDE;

private slots:
#if defined(QWEBENGINEPAGE_UNSUPPORTEDCONTENT)
    void handleUnsupportedContent(QNetworkReply *reply);
#endif
    void authenticationRequired(const QUrl &requestUrl, QAuthenticator *auth);
    void proxyAuthenticationRequired(const QUrl &requestUrl, QAuthenticator *auth, const QString &proxyHost);

private:
    friend class WebView;

    // set the webview mousepressedevent
    Qt::KeyboardModifiers m_keyboardModifiers;
    Qt::MouseButtons m_pressedButtons;
};
