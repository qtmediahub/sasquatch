#include "systemhelper.h"

#ifndef QT_NO_DBUS
#include <QtDBus>

class ConsoleKitInterface : public QDBusAbstractInterface
{
    Q_OBJECT
public:
    static inline const char *staticInterfaceName() { return "org.freedesktop.ConsoleKit"; }
    ConsoleKitInterface(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent = 0)
        : QDBusAbstractInterface(service, path, staticInterfaceName(), connection, parent)
    {}

public:
    inline QDBusPendingReply<> stop()
    {
        return asyncCall(QLatin1String("stop"));
    }

    inline QDBusPendingReply<> restart()
    {
        return asyncCall(QLatin1String("restart"));
    }
};

class UPowerInterface : public QDBusAbstractInterface
{
    Q_OBJECT
public:
    static inline const char *staticInterfaceName() { return "org.freedesktop.UPower"; }
    UPowerInterface(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent = 0)
        : QDBusAbstractInterface(service, path, staticInterfaceName(), connection, parent)
    {}

public:
    inline QDBusPendingReply<> suspend()
    {
        return asyncCall(QLatin1String("Suspend"));
    }

    inline QDBusPendingReply<> hibernate()
    {
        return asyncCall(QLatin1String("Hibernate"));
    }
};

#endif



SystemHelper::SystemHelper(QObject *parent) :
    QObject(parent)
{
}

void SystemHelper::shutdown()
{
#ifndef QT_NO_DBUS
    ConsoleKitInterface i("org.freedesktop.ConsoleKit.Manager", "/org/freedesktop/ConsoleKit/Manager", QDBusConnection::systemBus(), this);
    i.stop();
#else
    // TBD
#endif
}

void SystemHelper::restart()
{
#ifndef QT_NO_DBUS
    ConsoleKitInterface i("org.freedesktop.ConsoleKit.Manager", "/org/freedesktop/ConsoleKit/Manager", QDBusConnection::systemBus(), this);
    i.restart();
#else
    // TBD
#endif
}

void SystemHelper::suspend()
{
#ifndef QT_NO_DBUS
    UPowerInterface i("org.freedesktop.UPower", "/org/freedesktop/UPower", QDBusConnection::systemBus(), this);
    i.suspend();
#else
    // TBD
#endif
}

void SystemHelper::hibernate()
{
#ifndef QT_NO_DBUS
    UPowerInterface i("org.freedesktop.UPower", "/org/freedesktop/UPower", QDBusConnection::systemBus(), this);
    i.hibernate();
#else
    // TBD
#endif
}

#include "systemhelper.moc"
