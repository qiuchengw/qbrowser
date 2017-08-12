#pragma once

#include <QWidget>
#include "ui_workerstack.h"
#include "kstyledwidget.h"

class FQueryPanel;
class FArticleList;

class TabWidget;
class WebView;
class WorkerStack : public QTabWidget
{
    Q_OBJECT

public:
    WorkerStack(QWidget *parent, TabWidget* tab);
    ~WorkerStack();

public slots:
void onNewTab(WebView*);
void onCloseTab(WebView*);
void onTabsChanged(int);
void onOpenUrl(QString file, bool newtab);

private:
    Ui::WorkerStack ui;
    TabWidget* tab_ = nullptr;

    QStackedLayout *layout_ = nullptr;
    FQueryPanel* query_panel_ = nullptr;
};
