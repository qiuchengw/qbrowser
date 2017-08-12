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

typedef void* BROWSER_HANDLE;

extern "C" 
{
    BROWSER_EXPORT bool initBrowserSevice();

    // 创建一个新的窗口
    BROWSER_EXPORT BROWSER_HANDLE createBrowser();

    // cur_tab 是否复用当前的标签页
    BROWSER_EXPORT bool openUrl(BROWSER_HANDLE h, const QString& url, bool new_tab);
    
}
