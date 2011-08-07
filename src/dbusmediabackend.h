#ifndef DBUSMEDIABACKEND_H
#define DBUSMEDIABACKEND_H

#include "media-helper/mediabackendinterface.h"

#include <QtDBus>

class DBusMediaBackend : public MediaBackendInterface
{
    Q_OBJECT
public:
    explicit DBusMediaBackend(QObject *parent = 0);

signals:

public slots:
    Q_SCRIPTABLE virtual void loadUri(const QString &uri) { interface->call(__FUNCTION__, uri); }
    Q_SCRIPTABLE virtual void stop() { interface->call(__FUNCTION__); }
    Q_SCRIPTABLE virtual void pause(bool on = true) { interface->call(__FUNCTION__, on); }
    Q_SCRIPTABLE virtual void play() { interface->call(__FUNCTION__); }
    Q_SCRIPTABLE virtual void mute(bool on = true) { interface->call(__FUNCTION__, on); }
    Q_SCRIPTABLE virtual void setPosition(int position) { interface->call(__FUNCTION__, position); }
    Q_SCRIPTABLE virtual void setPosition(qreal position) { interface->call(__FUNCTION__, position); }

private:
    QDBusInterface *interface;
};

#endif // DBUSMEDIABACKEND_H
