#include "declarativeview.h"

DeclarativeView::DeclarativeView(QWidget *parent)
    : QDeclarativeView(parent),
      m_frameCount(0)
{
    startTimer(1000);
    connect(this, SIGNAL(statusChanged(QDeclarativeView::Status)), this, SLOT(handleStatusChanged(QDeclarativeView::Status)));
}

void DeclarativeView::setSource(const QUrl &url)
{
    m_url = url;
    QMetaObject::invokeMethod(this, "handleSourceChanged", Qt::QueuedConnection);
}

void DeclarativeView::paintEvent(QPaintEvent *event)
{
    m_frameTimer.restart();
    QGraphicsView::paintEvent(event);
    m_timeSigma += m_frameTimer.elapsed();
    ++m_frameCount;
}

void DeclarativeView::timerEvent(QTimerEvent *event)
{
    if (m_timeSigma) {
        int cap = 1000*m_frameCount/m_timeSigma;
        m_timeSigma = m_frameCount = 0;
        emit fpsCap(cap);
    }
    QDeclarativeView::timerEvent(event);
}

void DeclarativeView::handleSourceChanged()
{
    QDeclarativeView::setSource(m_url);
}

void DeclarativeView::handleStatusChanged(QDeclarativeView::Status status)
{
    if (status == QDeclarativeView::Ready) {
        activateWindow();
    }
}

