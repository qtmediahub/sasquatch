#include "rpc/rpcconnection.h"
#include "remotecontrol.h"

#include <QtGui>

RemoteControl::RemoteControl(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);
    m_connection = new RpcConnection(RpcConnection::Client);

    QSignalMapper *mapper = new QSignalMapper(this);
    #define MAPBUTTON(button, text) \
        mapper->setMapping(button, text); \
        connect(button, SIGNAL(clicked()), mapper, SLOT(map()))

    MAPBUTTON(ui.upButton, "up");
    MAPBUTTON(ui.downButton, "down");
    MAPBUTTON(ui.leftButton, "left");
    MAPBUTTON(ui.rightButton, "right");
    MAPBUTTON(ui.okButton, "ok");
    MAPBUTTON(ui.escButton, "esc");
    MAPBUTTON(ui.infoButton, "info");

    connect(mapper, SIGNAL(mapped(QString)), this, SLOT(sendButtonPress(QString)));
}

RemoteControl::~RemoteControl()
{
}

void RemoteControl::sendButtonPress(const QString &button)
{
    qDebug() << "Sending button press " << button;
    m_connection->call("qmhrpc.remoteControlButtonPressed(QString)", button);
}

