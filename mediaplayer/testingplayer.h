#ifndef TESTINGPLAYER_H
#define TESTINGPLAYER_H

#include "abstractmediaplayer.h"

class TestingPlayer : public AbstractMediaPlayer
{
    Q_OBJECT
public:
    explicit TestingPlayer(QObject *parent = 0) :
        AbstractMediaPlayer(parent) { /**/ }

signals:

public slots:
    void setSource(const QString &source) { qDebug() << __FUNCTION__ << source; }
    void stop() { qDebug() << __FUNCTION__; }
    void pause() { qDebug() << __FUNCTION__; }
    void resume() { qDebug() << __FUNCTION__; }
    void play() { qDebug() << __FUNCTION__; }
    void mute(bool on = true) { qDebug() << __FUNCTION__ << on; }
    void setPosition(int position) { qDebug() << __FUNCTION__ << position; }
    void setPositionPercent(qreal position) { qDebug() << __FUNCTION__ << position; }
    void setVolumePercent(qreal volume) { qDebug() << __FUNCTION__ << volume; }
};

#endif // TESTINGPLAYER_H
