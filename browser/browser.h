#pragma once


#include <QtCore/qglobal.h>

#ifndef BUILD_STATIC
# if defined(BROWSER_LIB)
#  define BROWSER_EXPORT Q_DECL_EXPORT
# else
#  define BROWSER_EXPORT Q_DECL_IMPORT
# endif
#else
# define BROWSER_EXPORT
#endif

#include "webpage.h"
#include "webview.h"

typedef void* BROWSER_HANDLE;
typedef std::function<void(BROWSER_HANDLE)> BrowserDestoryCallback;
typedef std::function<void(BROWSER_HANDLE)> CreateBrowserDelegater;

class BrowserServiceContext : public QObject {
    Q_OBJECT

public:
    //...

signals:
    // 新创建了标签页
    void onNewTab(WebView*);
    void onCloseTab(WebView*);
    void onTabsChanged(int);
    void onOpenUrl(QString file, bool newtab);
};

class BrowserWnd;
class BrowserFunctionPanel {
    friend class BrowserWnd;
public:
    BrowserFunctionPanel(const QString& name)
        :name_(name) {
    }

    QString name()const {
        return name_;
    }

    virtual QWidget* widget()const {
        Q_ASSERT(false);
        return nullptr;
    }

    virtual void onNewTab(WebView*) {

    }

    virtual void onCloseTab(WebView*) {

    }

    virtual void onTabActived(WebView*) {
    }

    // 当前活动的tab页面
    WebView* webView()const;

//     virtual void onUrlChanged(WebView*, const QString& url) {
//
//     }
private:
    void setBrowserWnd(BrowserWnd* wnd) {
        wnd_ = wnd;
    }

protected:
    BrowserWnd* wnd_ = nullptr;
    QString name_;
};

extern "C"
{
    BROWSER_EXPORT bool initBrowserSevice(BrowserServiceContext* ctx);

    // 创建一个新的窗口
    //	onDestroy 销毁窗口的通知
    BROWSER_EXPORT BROWSER_HANDLE createBrowser(BrowserDestoryCallback onDestroy, CreateBrowserDelegater createNew);

    // new_tab 是否打开一个新的标签页
    // bkgnd 是否后台打开窗口
    BROWSER_EXPORT WebView* openUrl(BROWSER_HANDLE h, const QString& url, bool new_tab, bool bkgnd);

    // 窗口级的功能面板
    BROWSER_EXPORT void addBrowserFunctionPanel(BROWSER_HANDLE h, BrowserFunctionPanel* panel);
}
