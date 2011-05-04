#ifndef SYSTEMHELPERDBUS_H
#define SYSTEMHELPERDBUS_H

#include <QObject>
#include <QtDBus>

#include "device.h"

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

class UDisksInterface : public QDBusAbstractInterface
{
    Q_OBJECT
public:
    static inline const char *staticInterfaceName() { return "org.freedesktop.UDisks"; }
    UDisksInterface(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent = 0)
        : QDBusAbstractInterface(service, path, staticInterfaceName(), connection, parent)
    {}

signals:
    void DeviceAdded(QDBusObjectPath path);
    void DeviceRemoved(QDBusObjectPath path);
};

class UDisksDeviceInterface : public QDBusAbstractInterface
{
    Q_OBJECT
public:
    static inline const char *staticInterfaceName() { return "org.freedesktop.UDisks.Device"; }
    UDisksDeviceInterface(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent = 0)
        : QDBusAbstractInterface(service, path, staticInterfaceName(), connection, parent)
    {}

    inline QString IdLabel()
    {
        QDBusReply<QString> reply = call(QLatin1String("IdLabel"));
        if (reply.error().isValid())
            return QString();
        else
            return reply.value();
    }

    inline QString IdUuid()
    {
        QDBusReply<QString> reply = call(QLatin1String("IdUuid"));
        if (reply.error().isValid())
            return QString();
        else
            return reply.value();
    }

    inline QString IdType()
    {
        QDBusReply<QString> reply = call(QLatin1String("IdType"));
        if (reply.error().isValid())
            return QString();
        else
            return reply.value();
    }

    inline QDBusPendingReply<> FilesystemMount()
    {
        QList<QVariant> argumentList;
        argumentList << QString();
        argumentList << QStringList();

        return asyncCallWithArgumentList(QLatin1String("FilesystemMount"), argumentList);
    }

    inline QDBusPendingReply<> FilesystemUnmount()
    {
        QList<QVariant> argumentList;
        argumentList << QStringList();

        return asyncCallWithArgumentList(QLatin1String("FilesystemUnmount"), argumentList);
    }

    inline QDBusPendingReply<> DriveEject()
    {
        QList<QVariant> argumentList;
        argumentList << QStringList();

        return asyncCallWithArgumentList(QLatin1String("DriveEject"), argumentList);
    }
};

class SystemHelperDBus : public QObject
{
    Q_OBJECT

public:
    SystemHelperDBus(QObject *parent = 0);

    ~SystemHelperDBus()
    {
        delete m_uDisks;
    }

    Device *getDeviceByPath(const QString &path)
    {
        if (m_devices.contains(path))
            return m_devices.value(path);
        return 0;
    }

public slots:
    void mount(const QString &path);
    void unmount(const QString &path);
    void eject(const QString &path);

    void newDevice(QDBusObjectPath path);
    void removeDevice(QDBusObjectPath path);

signals:
    void deviceAdded(QString device);
    void deviceRemoved(QString device);

private:
    UDisksInterface *m_uDisks;
    QHash<QString, Device*> m_devices;
};

#endif // SYSTEMHELPERDBUS_H
