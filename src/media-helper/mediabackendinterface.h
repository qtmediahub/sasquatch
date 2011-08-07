#ifndef MEDIAPLAYBACKINTERFACE_H
#define MEDIAPLAYBACKINTERFACE_H

#include <QObject>
#include <QDebug>

class MediaBackendInterface : public QObject
{
    Q_OBJECT
public:
    explicit MediaBackendInterface(QObject *parent = 0);

signals:

public slots:
    Q_SCRIPTABLE virtual void loadUri(const QString &uri) { Q_UNUSED(uri); qDebug() << __FUNCTION__ << "unimplemented"; }
    Q_SCRIPTABLE virtual void stop() { qDebug() << __FUNCTION__ << "unimplemented"; }
    Q_SCRIPTABLE virtual void pause(bool on = true) { Q_UNUSED(on); qDebug() << __FUNCTION__ << "unimplemented"; }
    Q_SCRIPTABLE virtual void play() { qDebug() << __FUNCTION__ << "unimplemented"; }
    Q_SCRIPTABLE virtual void mute(bool on = true) { Q_UNUSED(on); qDebug() << __FUNCTION__ << "unimplemented"; }
    Q_SCRIPTABLE virtual void setPosition(int position) { Q_UNUSED(position); qDebug() << __FUNCTION__ << "unimplemented"; }
    Q_SCRIPTABLE virtual void setPosition(qreal position) { Q_UNUSED(position); qDebug() << __FUNCTION__ << "unimplemented"; }
};

#endif // MEDIAPLAYBACKINTERFACE_H
