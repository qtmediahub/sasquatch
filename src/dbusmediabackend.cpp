#include "dbusmediabackend.h"

DBusMediaBackend::DBusMediaBackend(QObject *parent)
    : MediaBackendInterface(parent),
      interface(new QDBusInterface(QMH_HELPER_DBUS_SERVICENAME, "/", QString(), QDBusConnection::sessionBus(), this))
{
}
