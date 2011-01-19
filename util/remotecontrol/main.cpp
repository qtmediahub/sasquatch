#include <QtGui>

#include "remotecontrol.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    RemoteControl remoteControl;
    remoteControl.setWindowFlags(Qt::WindowStaysOnTopHint|Qt::X11BypassWindowManagerHint);
    remoteControl.adjustSize();
    remoteControl.show();

    return app.exec();
}

