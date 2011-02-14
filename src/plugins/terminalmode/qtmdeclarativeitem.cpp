#include "qtmdeclarativeitem.h"

#include <QGraphicsProxyWidget>

void QTmDeclarativeItem::initModule()
{
    qmlRegisterType<QTmDeclarativeItem>("QTerminalMode", 1, 0, "TerminalMode");
}

QTmDeclarativeItem::QTmDeclarativeItem(QDeclarativeItem *parent) :
    QDeclarativeItem(parent),
    mPW(0),
    mIsConnected(false)
{
    m_TerminalMode = new QTmClient(0);


    connect(m_TerminalMode, SIGNAL(upnpApplicationAdded(QTmApplication *)),
            this,           SLOT  (upnpApplicationAdded(QTmApplication *)));

    connect(m_TerminalMode, SIGNAL(upnpApplicationStatusChanged(QString, QTmGlobal::TmStatus)),
            this,           SLOT  (upnpApplicationStatusChanged(QString, QTmGlobal::TmStatus)));


    m_TerminalMode->networkStartObserver(true,false);

    //    connect(m_TerminalMode, SIGNAL(vncClientConnected(int)),      this, SLOT(vncConnected(int)));
    //    connect(m_TerminalMode, SIGNAL(vncClientDisconnected(int)),   this, SLOT(vncDisconnected(int)));
    //    connect(m_TerminalMode, SIGNAL(vncClientResized(QSize, int)), this, SLOT(vncResized(QSize, int)));

        connect(m_TerminalMode, SIGNAL(upnpApplicationStarted(QTmApplication *)),
                this,           SLOT  (upnpApplicationStarted(QTmApplication *)));
    //    connect(m_TerminalMode, SIGNAL(remoteServerDe`viceAdded(QTmRemoteServer*)),
    //            this,           SLOT  (remoteServerDeviceAdded(QTmRemoteServer*)));

    connect(m_TerminalMode, SIGNAL(networkDeviceDetected(QString, QHostAddress)), this, SLOT(deviceDetected(QString, QHostAddress)));
    connect(m_TerminalMode, SIGNAL(networkDeviceLost(QHostAddress)),              this, SLOT(deviceLost(QHostAddress)));

}

void QTmDeclarativeItem::upnpApplicationAdded(QTmApplication *application)
{
    qDebug() << "void QTmDeclarativeItem::upnpApplicationAdded(QTmApplication *application)";
    if(!mIsConnected) {
        m_TerminalMode->upnpLaunchApplication(application);
    }
}

void QTmDeclarativeItem::upnpApplicationStarted(QTmApplication *application)
{
    qDebug() << "void QTmDeclarativeItem::upnpApplicationStarted(QTmApplication *application)";
    mVNCid = m_TerminalMode->vncStartClient();
    QWidget *widget = m_TerminalMode->vncClientWidget(mVNCid);

    m_TerminalMode->vncConnectClient(application, mVNCid);
    mIsConnected = true;

    if(mPW == 0) {
        mPW = new QGraphicsProxyWidget(this);
    }
    mPW->setWidget(widget);
}

void QTmDeclarativeItem::deviceDetected(QString interfaceName, QHostAddress interface) {
    qDebug() << "MainApplication:" << "Terminal Mode Device detected" << interface.toString();
    if (interface.isInSubnet(QHostAddress("10.0.0.1"), 8)) {
        qDebug() << "MainApplication:" << "... Skipping local LAN";
        return;
    }
    //    if (m_interfaceList->contains(interface.toString()))
    //        return;

    int id = m_TerminalMode->upnpStartControlPoint();
//    m_interfaceList->insert(interface.toString(), id);
    m_TerminalMode->upnpConnectControlPoint(interfaceName, interface, id);
    //    repaint();
}

void QTmDeclarativeItem::deviceLost(QHostAddress interface)
{
    if(mPW != 0)
        mPW->setWidget(0);

    m_TerminalMode->upnpDisconnectControlPoint(mVNCid);
    m_TerminalMode->upnpStopControlPoint(mVNCid);
    m_TerminalMode->vncDisconnectClient(mVNCid);
    m_TerminalMode->vncStopClient(mVNCid);

    mIsConnected = false;
}

