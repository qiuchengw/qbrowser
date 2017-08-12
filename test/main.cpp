#include "stdafx.h"
#include "test.h"
#include <QtWidgets/QApplication>

int main(int argc, char **argv)
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    Q_INIT_RESOURCE(data);
    BrowserApplication application(argc, argv);
    if (!application.isTheOnlyBrowser())
        return 0;

    application.newMainWindow();
    return application.exec();
}

