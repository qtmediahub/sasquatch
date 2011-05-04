#include "systemhelper.h"

#ifndef QT_NO_DBUS
#include "systemhelperdbus.h"
#endif

SystemHelper::SystemHelper(QObject *parent) :
    QObject(parent)
{
#ifndef QT_NO_DBUS
    m_helper = new SystemHelperDBus(this);
    connect(m_helper, SIGNAL(deviceAdded(QString)), this, SIGNAL(deviceAdded(QString)));
    connect(m_helper, SIGNAL(deviceRemoved(QString)), this, SIGNAL(deviceRemoved(QString)));
#else
    // TBD
#endif
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

QObject *SystemHelper::getDeviceByPath(const QString &path)
{
#ifndef QT_NO_DBUS
    return qobject_cast<QObject*>(m_helper->getDeviceByPath(path));
#else
    return 0;
#endif
}


