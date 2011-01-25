#include <QtGui>

#include "remotecontrol.h"
#include "avahiservicebrowserview.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    RemoteControl remoteControl;
    remoteControl.setWindowFlags(Qt::WindowStaysOnTopHint|Qt::X11BypassWindowManagerHint);
    remoteControl.adjustSize();

    AvahiServiceBrowserView serviceBrowserView;
    QObject::connect(&serviceBrowserView, SIGNAL(serviceSelected(QHostAddress, int)),
                     &remoteControl, SLOT(connectToService(QHostAddress, int)));
    serviceBrowserView.show();

    return app.exec();
}

