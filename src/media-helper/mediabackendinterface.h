#ifndef MEDIAPLAYBACKINTERFACE_H
#define MEDIAPLAYBACKINTERFACE_H

#include <QObject>
#include <QDebug>

class MediaBackendInterface : public QObject
{
    Q_OBJECT
public:
    explicit MediaBackendInterface(QObject *parent = 0);
    virtual ~MediaBackendInterface() { /**/ }

signals:

public slots:
    Q_SCRIPTABLE virtual void loadUri(const QString &uri) = 0;
    Q_SCRIPTABLE virtual void stop() = 0;
    Q_SCRIPTABLE virtual void pause(bool on = true) = 0;
    Q_SCRIPTABLE virtual void play() = 0;
    Q_SCRIPTABLE virtual void mute(bool on = true) = 0;
    Q_SCRIPTABLE virtual void setPosition(int position) = 0;
    Q_SCRIPTABLE virtual void setPosition(qreal position) = 0;
};

#endif // MEDIAPLAYBACKINTERFACE_H
