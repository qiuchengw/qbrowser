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
    // �´����˱�ǩҳ
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

    // ��ǰ���tabҳ��
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

    // ����һ���µĴ���
    //	onDestroy ���ٴ��ڵ�֪ͨ
    BROWSER_EXPORT BROWSER_HANDLE createBrowser(BrowserDestoryCallback onDestroy, CreateBrowserDelegater createNew);

    // new_tab �Ƿ��һ���µı�ǩҳ
    // bkgnd �Ƿ��̨�򿪴���
    BROWSER_EXPORT WebView* openUrl(BROWSER_HANDLE h, const QString& url, bool new_tab, bool bkgnd);

    // ���ڼ��Ĺ������
    BROWSER_EXPORT void addBrowserFunctionPanel(BROWSER_HANDLE h, BrowserFunctionPanel* panel);
}
