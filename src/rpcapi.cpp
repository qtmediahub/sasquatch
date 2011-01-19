#include "rpcapi.h"
#include "frontend.h"

#include <QtGui>

RpcApi::RpcApi(Frontend *frontend)
    : QObject(frontend), m_frontend(frontend)
{
    setObjectName("qmhrpc");

    m_keyMap["ok"] = Qt::Key_Enter;
    m_keyMap["enter"] = Qt::Key_Enter;
    m_keyMap["return"] = Qt::Key_Return;
    m_keyMap["up"] = Qt::Key_Up;
    m_keyMap["down"] = Qt::Key_Down;
    m_keyMap["left"] = Qt::Key_Left;
    m_keyMap["right"] = Qt::Key_Right;
    m_keyMap["back"] = Qt::Key_Escape;
    m_keyMap["esc"] = Qt::Key_Escape;
    m_keyMap["info"] = Qt::Key_I;
}

RpcApi::~RpcApi()
{
}

void RpcApi::remoteControlButtonPressed(const QString &button)
{
    if (!m_keyMap.contains(button))
        return;
    int key = m_keyMap[button];
    qDebug() << "Received button " << button;
    QKeyEvent keyPress(QEvent::KeyPress, key, Qt::NoModifier);
    qApp->sendEvent(m_frontend->centralWidget(), &keyPress);
    QKeyEvent keyRelease(QEvent::KeyRelease, key, Qt::NoModifier);
    qApp->sendEvent(m_frontend->centralWidget(), &keyRelease);
}

