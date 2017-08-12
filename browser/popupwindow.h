#pragma once


class WebView;
class PopupWindow : public QWidget {
    Q_OBJECT
public:
    PopupWindow(QWebEngineProfile *profile);

    QWebEnginePage* page() const;

private slots:
void setUrl(const QUrl &url);

void adjustGeometry(const QRect &newGeometry);

private:
    QLineEdit *m_addressBar;
    WebView *m_view;
};

