#include <QtGui>

#include "remotecontrol.h"
#include "avahiservicebrowserview.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    RemoteControl remoteControl;
#ifndef Q_WS_MAEMO_5
    remoteControl.setWindowFlags(Qt::WindowStaysOnTopHint|Qt::X11BypassWindowManagerHint);
    remoteControl.adjustSize();
#endif
    QObject::connect(&remoteControl, SIGNAL(disconnected()), &remoteControl, SLOT(hide()));

    AvahiServiceBrowserView serviceBrowserView;
    QObject::connect(&serviceBrowserView, SIGNAL(serviceSelected(QHostAddress, int)),
                     &remoteControl, SLOT(connectToService(QHostAddress, int)));
    QObject::connect(&remoteControl, SIGNAL(disconnected()), &serviceBrowserView, SLOT(show()));
    serviceBrowserView.show();

    return app.exec();
}

