#include "systemhelperdbus.h"

SystemHelperDBus::SystemHelperDBus(QObject *parent) :
    QObject(parent)
{
    m_uDisks = new UDisksInterface("org.freedesktop.UDisks", "/org/freedesktop/UDisks", QDBusConnection::systemBus(), this);
    connect(m_uDisks, SIGNAL(DeviceAdded(QDBusObjectPath)), this, SLOT(newDevice(QDBusObjectPath)));
    connect(m_uDisks, SIGNAL(DeviceRemoved(QDBusObjectPath)), this, SLOT(removeDevice(QDBusObjectPath)));
}

void SystemHelperDBus::mount(const QString &path)
{
    UDisksDeviceInterface di("org.freedesktop.UDisks", path, QDBusConnection::systemBus(), this);
    if (!di.isValid())
        return;
    di.FilesystemMount();
}

void SystemHelperDBus::unmount(const QString &path)
{
    UDisksDeviceInterface di("org.freedesktop.UDisks", path, QDBusConnection::systemBus(), this);
    if (!di.isValid())
        return;
    di.FilesystemUnmount();
}

void SystemHelperDBus::eject(const QString &path)
{
    UDisksDeviceInterface di("org.freedesktop.UDisks", path, QDBusConnection::systemBus(), this);
    if (!di.isValid())
        return;
    di.DriveEject();
}

void SystemHelperDBus::newDevice(QDBusObjectPath path)
{
    UDisksDeviceInterface di("org.freedesktop.UDisks", path.path(), QDBusConnection::systemBus(), this);
    if (!di.isValid())
        return;

    // TODO get type form di.idType and others
    Device::DeviceType type = Device::Undefined;

    Device *d = new Device(path.path(), type, di.IdLabel(), di.IdUuid(), this);
    m_devices.insert(path.path(), d);

    emit deviceAdded(path.path());
}

void SystemHelperDBus::removeDevice(QDBusObjectPath path)
{
    // TODO cleanup device
    emit deviceRemoved(path.path());
}

