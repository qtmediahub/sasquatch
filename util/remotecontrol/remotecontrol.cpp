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
    MAPBUTTON(ui.okButton, ActionMapper::Enter);
    MAPBUTTON(ui.escButton, ActionMapper::Menu);
    MAPBUTTON(ui.infoButton, ActionMapper::Context);

    connect(mapper, SIGNAL(mapped(int)), this, SLOT(sendButtonPress(int)));

    m_connection = new RpcConnection(RpcConnection::Client, this);
    connect(m_connection, SIGNAL(clientConnected()), this, SLOT(enableButtons()));
    connect(m_connection, SIGNAL(clientDisconnected()), this, SIGNAL(disconnected()));
    connect(ui.quitButton, SIGNAL(clicked()), m_connection, SLOT(disconnectFromHost()));

    ui.buttonsFrame->setEnabled(false);
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
    qDebug() << "Connecting to " << address.toString() << ":" << port;
    m_connection->connectToHost(address, port);
}

void RemoteControl::closeEvent(QCloseEvent *event)
{
    m_connection->disconnectFromHost();
}

void RemoteControl::enableButtons()
{
    ui.buttonsFrame->setEnabled(true);
}

