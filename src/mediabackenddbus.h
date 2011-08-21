#ifndef DBUSMEDIABACKEND_H
#define DBUSMEDIABACKEND_H

#include "mediabackendinterface.h"

#ifndef NO_DBUS
#include <QtDBus>
#endif

class DBusMediaBackend : public MediaBackendInterface
{
    Q_OBJECT
public:
    explicit DBusMediaBackend(QObject *parent = 0);

signals:

public slots:
#ifndef NO_DBUS
    void loadUri(const QString &uri) { interface->call(__FUNCTION__, uri); }
    void stop() { interface->call(__FUNCTION__); }
    void pause() { interface->call(__FUNCTION__); }
    void resume() { interface->call(__FUNCTION__); }
    void play() { interface->call(__FUNCTION__); }
    void mute(bool on = true) { interface->call(__FUNCTION__, on); }
    void setPosition(int position) { interface->call(__FUNCTION__, position); }
    void setPositionPercent(qreal position) { interface->call(__FUNCTION__, position); }
    void setVolumePercent(qreal volume)  { interface->call(__FUNCTION__, volume); }
private:
    QDBusInterface *interface;
#endif
};

#endif // DBUSMEDIABACKEND_H
