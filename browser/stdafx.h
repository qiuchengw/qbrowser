#pragma once

#include <QtWidgets>

#include <QtCore/QEvent>
#include <QtCore/QMimeData>

#include <QAuthenticator>
#include <QNetworkCookieJar>
#include <QNetworkCookie>
#include <QWebEngineDownloadItem>
#include <QWebEnginePage>
#include <QWebEngineScript>
#include <QWebEngineProfile>
#include <QLocalServer>
#include <QWebEngineSettings>
#include <QTextStream>
#include <QNetworkProxy>
#include <QWebEngineScriptCollection>
#include <QPrintPreviewDialog>
#include <QPrinter>
#include <QPrintDialog>
#include <QWebEngineHistoryItem>
#include <QPageSetupDialog>
#include <QWebEngineFullScreenRequest>
#include <QUiLoader>
#include <QWebEngineView>
#include <QWebEngineContextMenuData>

#include "kutil/macro.h"
#include "kutil/kcommon.h"
#include "kutil/singleton.h"
#include "kutil/misc.h"
#include "kutil/widget_helper.h"

#define FV_APP_NAME "ARTICLE_MAN"
#define FV_APP_VERSION "1.0"

#include "deps/qt-rest-client/rest-client/restclient.h"
#include "deps/qt-rest-client/rest-client/requesterror.h"
#include "deps/qt-rest-client/rest-client/resturlbuilder.h"
#include "deps/rjson/inc.h"

#include "xusr/xusr.h"
#include "xusr/userinfo.h"
#include "xusr/krestclient.h"
#include "knet/net.h"
#include "../common.h"

#define QSL QStringLiteral

