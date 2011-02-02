#include <QtGui>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("QMH Remote Control");
    app.setOrganizationName("Nokia");

    MainWindow mainWindow;
#if defined(Q_OS_SYMBIAN) || defined(Q_WS_MAEMO5)
    mainWindow.showMaximized();
#else
    mainWindow.show();
#endif

    return app.exec();
}

