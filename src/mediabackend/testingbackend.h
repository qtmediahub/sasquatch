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
    void loadUri(const QString &uri) { qDebug() << __FUNCTION__ << uri; }
    void stop() { qDebug() << __FUNCTION__; }
    void pause(bool on = true) { qDebug() << __FUNCTION__ << on; }
    void play() { qDebug() << __FUNCTION__; }
    void mute(bool on = true) { qDebug() << __FUNCTION__ << on; }
    void setPosition(int position) { qDebug() << __FUNCTION__ << position; }
    void setPosition(qreal position) { qDebug() << __FUNCTION__ << position; }
    void setVolume(qreal volume) { qDebug() << __FUNCTION__ << volume; }
};

#endif // TESTINGBACKEND_H
