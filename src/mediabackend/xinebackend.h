#ifndef XINEBACKEND_H
#define XINEBACKEND_H

#include "mediabackendinterface.h"

class XineBackend : public MediaBackendInterface
{
    //Bare bones reference backend (audio only)
    Q_OBJECT
public:
    explicit XineBackend(QObject *parent = 0);
    ~XineBackend();

signals:

public slots:
    void loadUri(const QString &uri);
    void stop();
    void pause();
    void resume();
    void play();
    void mute(bool on = true);
    void setPosition(int position);
    void setPositionPercent(qreal position);
    void setVolumePercent(qreal volume);
private:
    class Private;
    Private *d;
};

#endif // XINEBACKEND_H
