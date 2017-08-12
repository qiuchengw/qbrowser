#include "stdafx.h"
#include "browser.h"
#include "browserservice.h"

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

    bool openUrl(BROWSER_HANDLE h, const QString& url, bool new_tab) {
        browser(h)->loadPage(url, new_tab);
        return true;
    }


}
