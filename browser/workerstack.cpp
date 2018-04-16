#include "stdafx.h"
#include "workerstack.h"
#include "tabwidget.h"
#include "webview.h"

WorkerStack::WorkerStack(QWidget *parent, TabWidget* tab)
    : QTabWidget(parent), tab_(tab) {
    Q_ASSERT(tab);

    // setFixedWidth(320);
    setMinimumWidth(200);
    setStyleSheet("KStyledWidget{border-left:1px solid #828790;border-bottom:1px solid #828790; }");

    if (tab_) {
        // ¼à¿Øtab±ä»¯
        connect(tab_, &TabWidget::tabCreated, this, &WorkerStack::onNewTab);
        connect(tab_, &TabWidget::tabClosed, this, &WorkerStack::onCloseTab);
        connect(tab_, &TabWidget::currentChanged, this, &WorkerStack::onTabsChanged);
    }
}

WorkerStack::~WorkerStack() {
}

void WorkerStack::addPanel(BrowserFunctionPanel* panel) {
    int idx = addTab(panel->widget(), panel->name());
    panels_.insert(idx, panel);
}

void WorkerStack::onNewTab(WebView* w) {
    for (BrowserFunctionPanel* p : panels_.values()) {
        p->onNewTab(w);
    }
}

void WorkerStack::onCloseTab(WebView* w) {
    for (BrowserFunctionPanel* p : panels_.values()) {
        p->onCloseTab(w);
    }
}

void WorkerStack::onTabsChanged(int) {
    if (WebView* w = tab_->currentWebView()) {
        for (BrowserFunctionPanel* p : panels_.values()) {
            p->onTabActived(w);
        }
    }
}

void WorkerStack::onOpenUrl(QString file, bool newtab) {
    if (newtab) {
        if (WebView* w = tab_->newTab(true)) {
            w->setUrl(file);
        }
    } else {
        if (WebView* w = tab_->currentWebView()) {
            w->setUrl(file);
        }
    }
}
