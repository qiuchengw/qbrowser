#pragma once

#include <QTabWidget>

class FQueryPanel;
class FArticleList;

class TabWidget;
class WebView;
class WorkerStack : public QTabWidget {
    Q_OBJECT

public:
    WorkerStack(QWidget *parent, TabWidget* tab);
    ~WorkerStack();

    void addPanel(BrowserFunctionPanel* panel);

public slots:
    void onNewTab(WebView*);
    void onCloseTab(WebView*);
    void onTabsChanged(int);
    void onOpenUrl(QString file, bool newtab);

private:
    TabWidget* tab_ = nullptr;
    QMap<int, BrowserFunctionPanel*> panels_;
};
