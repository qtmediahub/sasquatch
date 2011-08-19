#include "mediabackenddbus.h"

DBusMediaBackend::DBusMediaBackend(QObject *parent)
    : MediaBackendInterface(parent)
#ifndef NO_DBUS
     , interface(new QDBusInterface(QMH_HELPER_DBUS_SERVICENAME, "/", QString(), QDBusConnection::sessionBus(), this))
#endif
{
}
