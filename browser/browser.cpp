#include "stdafx.h"
#include "browser.h"
#include "browserservice.h"
#include "browserwnd.h"

inline BrowserWnd* browser(BROWSER_HANDLE h) {
    return reinterpret_cast<BrowserWnd*>(h);
}

extern "C" {

    BROWSER_HANDLE createBrowser(){
        return BrowserService::instance()->newBroswer();
    }

    bool initBrowserSevice(){
        return true;
    }

    WebView* openUrl(BROWSER_HANDLE h, const QString& url, bool new_tab, bool bkgnd) {
        return browser(h)->loadUrl(url, new_tab, bkgnd);
    }
}
