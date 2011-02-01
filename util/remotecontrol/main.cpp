#include <QtGui>

#include "remotecontrol.h"
#ifndef QMH_NO_AVAHI
#include "avahiservicebrowserview.h"
#else
#include "staticservicebrowserview.h"
#endif

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    RemoteControl remoteControl;
#ifndef Q_WS_MAEMO_5
    remoteControl.setWindowFlags(Qt::WindowStaysOnTopHint|Qt::X11BypassWindowManagerHint);
    remoteControl.adjustSize();
#endif
    QObject::connect(&remoteControl, SIGNAL(disconnected()), &remoteControl, SLOT(hide()));

#ifndef QMH_NO_AVAHI
    AvahiServiceBrowserView serviceBrowserView;
#else
    StaticServiceBrowserView serviceBrowserView;
#endif

    QObject::connect(&serviceBrowserView, SIGNAL(serviceSelected(QHostAddress, int)),
                     &remoteControl, SLOT(connectToService(QHostAddress, int)));
#ifdef Q_OS_SYMBIAN
    QObject::connect(&remoteControl, SIGNAL(disconnected()), &serviceBrowserView, SLOT(showFullScreen()));
    serviceBrowserView.showFullScreen();
#else
    QObject::connect(&remoteControl, SIGNAL(disconnected()), &serviceBrowserView, SLOT(show()));
    serviceBrowserView.show();
#endif

    return app.exec();
}

