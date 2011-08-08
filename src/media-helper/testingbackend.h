#ifndef TESTINGBACKEND_H
#define TESTINGBACKEND_H

#include "mediabackendinterface.h"

class TestingBackend : public MediaBackendInterface
{
    Q_OBJECT
public:
    explicit TestingBackend(QObject *parent = 0) :
        MediaBackendInterface(parent) { /**/ }

signals:

public slots:
    void loadUri(const QString &uri) { Q_UNUSED(uri); qDebug() << __FUNCTION__ << uri; }
    void stop() { qDebug() << __FUNCTION__; }
    void pause(bool on = true) { qDebug() << __FUNCTION__ << on; }
    void play() { qDebug() << __FUNCTION__; }
    void mute(bool on = true) { qDebug() << __FUNCTION__ << on; }
    void setPosition(int position) { Q_UNUSED(position); qDebug() << __FUNCTION__; }
    void setPosition(qreal position) { Q_UNUSED(position); qDebug() << __FUNCTION__; }
};

#endif // TESTINGBACKEND_H
