#include "stdafx.h"
#include "test.h"
#include <QtWidgets/QApplication>

#include "../browser/browser.h"
#pragma comment(lib, "browser.lib")


void test_browser() {
    BROWSER_HANDLE h = createBrowser();
}

int main(int argc, char **argv) {
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication app(argc, argv);
    // Q_INIT_RESOURCE(data);

    test_browser();

    return app.exec();
}

