#ifndef XINEBACKEND_H
#define XINEBACKEND_H

#include "mediabackendinterface.h"

class XineBackend : public MediaBackendInterface
{
    Q_OBJECT
public:
    explicit XineBackend(QObject *parent = 0);
    ~XineBackend();

signals:

public slots:
    void loadUri(const QString &uri);
    void stop();
    void pause(bool on = true);
    void play();
    void mute(bool on = true);
    void setPosition(int position);
    void setPosition(qreal position);
private:
    class Private;
    Private *d;
};

#endif // XINEBACKEND_H
