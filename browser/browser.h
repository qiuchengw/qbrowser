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

extern "C" 
{
    BROWSER_EXPORT bool initBrowserSevice();

    // 创建一个新的窗口
    BROWSER_EXPORT BROWSER_HANDLE createBrowser();

    // new_tab 是否打开一个新的标签页
    // bkgnd 是否后台打开窗口
    BROWSER_EXPORT WebView* openUrl(BROWSER_HANDLE h, const QString& url, bool new_tab, bool bkgnd);
}
