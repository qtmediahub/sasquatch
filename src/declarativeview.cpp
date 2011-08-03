#include "declarativeview.h"

DeclarativeView::DeclarativeView(QWidget *parent)
    : QDeclarativeView(parent),
      frameCount(0)
{
    startTimer(1000);
    connect(this, SIGNAL(statusChanged(QDeclarativeView::Status)), this, SLOT(handleStatusChanged(QDeclarativeView::Status)));
}

void DeclarativeView::setSource(const QUrl &url)
{
    this->url = url;
    QMetaObject::invokeMethod(this, "handleSourceChanged", Qt::QueuedConnection);
}

void DeclarativeView::paintEvent(QPaintEvent *event)
{
    frameTimer.restart();
    QGraphicsView::paintEvent(event);
    timeSigma += frameTimer.elapsed();
    ++frameCount;
}

void DeclarativeView::timerEvent(QTimerEvent *event)
{
    if (timeSigma) {
        int cap = 1000*frameCount/timeSigma;
        timeSigma = frameCount = 0;
        emit fpsCap(cap);
    }
    QDeclarativeView::timerEvent(event);
}

void DeclarativeView::handleSourceChanged()
{
    QDeclarativeView::setSource(url);
}

void DeclarativeView::handleStatusChanged(QDeclarativeView::Status status)
{
    if (status == QDeclarativeView::Ready) {
        activateWindow();
    }
}

