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
    void loadUri(const QString &uri) { interface->call(__FUNCTION__, uri); }
    void stop() { interface->call(__FUNCTION__); }
    void pause(bool on = true) { interface->call(__FUNCTION__, on); }
    void play() { interface->call(__FUNCTION__); }
    void mute(bool on = true) { interface->call(__FUNCTION__, on); }
    void setPosition(int position) { interface->call(__FUNCTION__, position); }
    void setPosition(qreal position) { interface->call(__FUNCTION__, position); }
    void setVolume(qreal volume)  { interface->call(__FUNCTION__, volume); }
private:
    QDBusInterface *interface;
};

#endif // DBUSMEDIABACKEND_H
