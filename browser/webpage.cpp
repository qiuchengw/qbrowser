#include "stdafx.h"
#include "webpage.h"
#include "browserservice.h"
#include "browserwnd.h"
#include "ui_passworddialog.h"
#include "ui_proxy.h"
#include "popupwindow.h"
#include "tabwidget.h"
#include "webview.h"

WebPage::WebPage(QWebEngineProfile *profile, QObject *parent)
    : QWebEnginePage(profile, parent)
    , m_keyboardModifiers(Qt::NoModifier)
    , m_pressedButtons(Qt::NoButton)
{
#if defined(QWEBENGINEPAGE_SETNETWORKACCESSMANAGER)
    setNetworkAccessManager(BrowserService::networkAccessMan());
#endif
#if defined(QWEBENGINEPAGE_UNSUPPORTEDCONTENT)
    connect(this, SIGNAL(unsupportedContent(QNetworkReply*)),
        this, SLOT(handleUnsupportedContent(QNetworkReply*)));
#endif
    connect(this, SIGNAL(authenticationRequired(const QUrl &, QAuthenticator*)),
        SLOT(authenticationRequired(const QUrl &, QAuthenticator*)));
    connect(this, SIGNAL(proxyAuthenticationRequired(const QUrl &, QAuthenticator *, const QString &)),
        SLOT(proxyAuthenticationRequired(const QUrl &, QAuthenticator *, const QString &)));
}

BrowserWnd *WebPage::mainWindow()
{
    QObject *w = this->parent();
    while (w) {
        if (BrowserWnd *mw = qobject_cast<BrowserWnd*>(w))
            return mw;
        w = w->parent();
    }
    return BrowserService::instance()->browser();
}

bool WebPage::certificateError(const QWebEngineCertificateError &error)
{
    if (error.isOverridable()) {
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText(error.errorDescription());
        msgBox.setInformativeText(tr("If you wish so, you may continue with an unverified certificate. "
            "Accepting an unverified certificate means "
            "you may not be connected with the host you tried to connect to.\n"
            "Do you wish to override the security check and continue?"));
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::No);
        return msgBox.exec() == QMessageBox::Yes;
    }
    QMessageBox::critical(view(), tr("Certificate Error"), error.errorDescription(), QMessageBox::Ok, QMessageBox::NoButton);
    return false;
}


// #include "webview.moc"

QWebEnginePage *WebPage::createWindow(QWebEnginePage::WebWindowType type)
{
    if (type == QWebEnginePage::WebBrowserTab) {
        return mainWindow()->tabWidget()->newTab()->page();
    }
    else if (type == QWebEnginePage::WebBrowserBackgroundTab) {
        return mainWindow()->tabWidget()->newTab(false)->page();
    }
    else if (type == QWebEnginePage::WebBrowserWindow) {
        BrowserService::instance()->newBroswer();
        BrowserWnd *mainWindow = BrowserService::instance()->browser();
        return mainWindow->currentTab()->page();
    }
    else {
        PopupWindow *popup = new PopupWindow(profile());
        popup->setAttribute(Qt::WA_DeleteOnClose);
        popup->show();
        return popup->page();
    }
}

#if !defined(QT_NO_UITOOLS)
QObject *WebPage::createPlugin(const QString &classId, const QUrl &url, const QStringList &paramNames, const QStringList &paramValues)
{
    Q_UNUSED(url);
    Q_UNUSED(paramNames);
    Q_UNUSED(paramValues);
    QUiLoader loader;
    return loader.createWidget(classId, view());
}
#endif // !defined(QT_NO_UITOOLS)

#if defined(QWEBENGINEPAGE_UNSUPPORTEDCONTENT)
void WebPage::handleUnsupportedContent(QNetworkReply *reply)
{
    QString errorString = reply->errorString();

    if (m_loadingUrl != reply->url()) {
        // sub resource of this page
        qWarning() << "Resource" << reply->url().toEncoded() << "has unknown Content-Type, will be ignored.";
        reply->deleteLater();
        return;
    }

    if (reply->error() == QNetworkReply::NoError && !reply->header(QNetworkRequest::ContentTypeHeader).isValid()) {
        errorString = "Unknown Content-Type";
    }

    QFile file(QLatin1String(":/notfound.html"));
    bool isOpened = file.open(QIODevice::ReadOnly);
    Q_ASSERT(isOpened);
    Q_UNUSED(isOpened)

        QString title = tr("Error loading page: %1").arg(reply->url().toString());
    QString html = QString(QLatin1String(file.readAll()))
        .arg(title)
        .arg(errorString)
        .arg(reply->url().toString());

    QBuffer imageBuffer;
    imageBuffer.open(QBuffer::ReadWrite);
    QIcon icon = view()->style()->standardIcon(QStyle::SP_MessageBoxWarning, 0, view());
    QPixmap pixmap = icon.pixmap(QSize(32, 32));
    if (pixmap.save(&imageBuffer, "PNG")) {
        html.replace(QLatin1String("IMAGE_BINARY_DATA_HERE"),
            QString(QLatin1String(imageBuffer.buffer().toBase64())));
    }

    QList<QWebEngineFrame*> frames;
    frames.append(mainFrame());
    while (!frames.isEmpty()) {
        QWebEngineFrame *frame = frames.takeFirst();
        if (frame->url() == reply->url()) {
            frame->setHtml(html, reply->url());
            return;
        }
        QList<QWebEngineFrame *> children = frame->childFrames();
        foreach(QWebEngineFrame *frame, children)
            frames.append(frame);
    }
    if (m_loadingUrl == reply->url()) {
        mainFrame()->setHtml(html, reply->url());
    }
}
#endif

void WebPage::authenticationRequired(const QUrl &requestUrl, QAuthenticator *auth)
{
    BrowserWnd *mainWindow = BrowserService::instance()->browser();

    QDialog dialog(mainWindow);
    dialog.setWindowFlags(Qt::Sheet);

    Ui::PasswordDialog passwordDialog;
    passwordDialog.setupUi(&dialog);

    passwordDialog.iconLabel->setText(QString());
    passwordDialog.iconLabel->setPixmap(mainWindow->style()->standardIcon(QStyle::SP_MessageBoxQuestion, 0, mainWindow).pixmap(32, 32));

    QString introMessage = tr("<qt>Enter username and password for \"%1\" at %2</qt>");
    introMessage = introMessage.arg(auth->realm()).arg(requestUrl.toString().toHtmlEscaped());
    passwordDialog.introLabel->setText(introMessage);
    passwordDialog.introLabel->setWordWrap(true);

    if (dialog.exec() == QDialog::Accepted) {
        auth->setUser(passwordDialog.userNameLineEdit->text());
        auth->setPassword(passwordDialog.passwordLineEdit->text());
    }
    else {
        // Set authenticator null if dialog is cancelled
        *auth = QAuthenticator();
    }
}

void WebPage::proxyAuthenticationRequired(const QUrl &requestUrl, QAuthenticator *auth, const QString &proxyHost)
{
    Q_UNUSED(requestUrl);
    BrowserWnd *mainWindow = BrowserService::instance()->browser();

    QDialog dialog(mainWindow);
    dialog.setWindowFlags(Qt::Sheet);

    Ui::ProxyDialog proxyDialog;
    proxyDialog.setupUi(&dialog);

    proxyDialog.iconLabel->setText(QString());
    proxyDialog.iconLabel->setPixmap(mainWindow->style()->standardIcon(QStyle::SP_MessageBoxQuestion, 0, mainWindow).pixmap(32, 32));

    QString introMessage = tr("<qt>Connect to proxy \"%1\" using:</qt>");
    introMessage = introMessage.arg(proxyHost.toHtmlEscaped());
    proxyDialog.introLabel->setText(introMessage);
    proxyDialog.introLabel->setWordWrap(true);

    if (dialog.exec() == QDialog::Accepted) {
        auth->setUser(proxyDialog.userNameLineEdit->text());
        auth->setPassword(proxyDialog.passwordLineEdit->text());
    }
    else {
        // Set authenticator null if dialog is cancelled
        *auth = QAuthenticator();
    }
}
