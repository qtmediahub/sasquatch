#include "inputcontext.h"

#include <QApplication>
#include <QtDebug>

#define DEBUG if (1) qDebug() << __PRETTY_FUNCTION__

InputContext::InputContext(QObject *parent) :
    QInputContext(parent),
    m_composing(false)
{
    qApp->setInputContext(this);
}

bool InputContext::filterEvent(const QEvent *event)
{
    if (event->type() == QEvent::RequestSoftwareInputPanel) {
        m_composing = true;
        emit inputMethodStartRequested();
        DEBUG << "request Input Panel";
        return true;
    }

    if (event->type() == QEvent::CloseSoftwareInputPanel) {
        m_composing = false;
        emit inputMethodStopRequested();
        DEBUG << "request close Input Panel";
        return true;
    }

    return false;
}

void InputContext::reset()
{
    DEBUG;
    m_composing = false;
    emit inputMethodStopRequested();
}
