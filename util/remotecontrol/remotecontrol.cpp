#include "rpc/rpcconnection.h"
#include "remotecontrol.h"
#include "actionmapper.h"

#include <QtGui>

RemoteControl::RemoteControl(QWidget *parent)
    : QWidget(parent), m_connection(0)
{
    ui.setupUi(this);

    QSignalMapper *mapper = new QSignalMapper(this);
    #define MAPBUTTON(button, action) \
        mapper->setMapping(button, action); \
        connect(button, SIGNAL(clicked()), mapper, SLOT(map()))

    MAPBUTTON(ui.upButton, ActionMapper::Up);
    MAPBUTTON(ui.downButton, ActionMapper::Down);
    MAPBUTTON(ui.leftButton, ActionMapper::Left);
    MAPBUTTON(ui.rightButton, ActionMapper::Right);
    MAPBUTTON(ui.okButton, ActionMapper::Forward);
    MAPBUTTON(ui.escButton, ActionMapper::Back);
    MAPBUTTON(ui.infoButton, ActionMapper::Context);

    connect(mapper, SIGNAL(mapped(int)), this, SLOT(sendButtonPress(int)));
}

RemoteControl::~RemoteControl()
{
}

void RemoteControl::sendButtonPress(int action)
{
    m_connection->call("qmhrpc.takeAction(double)", static_cast<double>(action));
}

void RemoteControl::connectToService(const QHostAddress &address, int port)
{
    if (m_connection) {
        qDebug() << "Already connected";
        return;
    }

    show();
    qDebug() << "Connecting to " << address.toString() << ":" << port;
    m_connection = new RpcConnection(RpcConnection::Client, address, port);
}

