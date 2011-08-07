/****************************************************************************

This file is part of the QtMediaHub project on http://www.qtmediahub.com

Copyright (c) 2011 Nokia Corporation and/or its subsidiary(-ies).*
All rights reserved.

Contact:  Girish Ramakrishnan girish@forwardbias.in
Contact:  Nokia Corporation donald.carr@nokia.com
Contact:  Nokia Corporation johannes.zellner@nokia.com

You may use this file under the terms of the BSD license as follows:

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
* Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
* Neither the name of Nokia Corporation and its Subsidiary(-ies) nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

****************************************************************************/

#ifndef DEVICE_H
#define DEVICE_H

#include <QObject>

#ifndef QT_NO_DBUS

#include <QDBusAbstractInterface>
#include <QDBusPendingReply>
#include <QDBusReply>

class UDisksDeviceInterface : public QDBusAbstractInterface
{
    Q_OBJECT

public:
    static inline const char *staticInterfaceName() { return "org.freedesktop.UDisks.Device"; }
    UDisksDeviceInterface(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent = 0)
        : QDBusAbstractInterface(service, path, staticInterfaceName(), connection, parent)
    {
    }

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

    inline bool DeviceIsPartition()
    {
        QDBusMessage message = QDBusMessage::createMethodCall(service(), path(), QLatin1String("org.freedesktop.DBus.Properties"), QLatin1String("Get"));
        QList<QVariant> arguments;
        arguments << staticInterfaceName() << "DeviceIsPartition";
        message.setArguments(arguments);
        QDBusReply<QVariant> reply = connection().call(message);
        if (reply.error().isValid())
            return false;
        return reply.value().toBool();
    }

    inline QString DeviceMountPath()
    {
        QDBusMessage message = QDBusMessage::createMethodCall(service(), path(), QLatin1String("org.freedesktop.DBus.Properties"), QLatin1String("Get"));
        QList<QVariant> arguments;
        arguments << staticInterfaceName() << "DeviceMountPaths";
        message.setArguments(arguments);
        QDBusReply<QVariant> reply = connection().call(message);
        if (reply.error().isValid())
            return QString();

        QStringList v = reply.value().toStringList();
        if (v.count() > 0)
            return v.first();
        return QString();
    }

signals:
    void Changed();

};
#endif

class Device : public QObject
{
    Q_OBJECT
    Q_ENUMS(DeviceType)
    Q_PROPERTY(QString path READ path NOTIFY changed)
    Q_PROPERTY(DeviceType type READ type NOTIFY changed)
    Q_PROPERTY(QString label READ label NOTIFY changed)
    Q_PROPERTY(QString uuid READ uuid NOTIFY changed)
    Q_PROPERTY(bool valid READ valid NOTIFY changed)
    Q_PROPERTY(bool isPartition READ isPartition NOTIFY changed)
    Q_PROPERTY(QString mountPoint READ mountPoint NOTIFY changed)

public:
    enum DeviceType { Undefined, UsbDrive, Dvd, AudioCd, TypeCount };

    Device(const QString &p, QObject *parent = 0);

    QString path() const { return m_path; }
    DeviceType type() const { return m_type; }
    QString label() const { return m_label; }
    QString uuid() const { return m_uuid; }
    bool valid() const { return m_valid; }
    bool isPartition() const { return m_isPartition; }
    QString mountPoint() const { return m_mountPoint; }

signals:
    void changed();

public slots:
    void mount();
    void unmount();
    void eject();

private slots:
    void deviceChanged();

private:
    QString m_path;
    DeviceType m_type;
    QString m_label;
    QString m_uuid;
    bool m_valid;
    bool m_isPartition;
    QString m_mountPoint;

#ifndef QT_NO_DBUS
    UDisksDeviceInterface *m_deviceInterface;
#endif
};

#endif // DEVICE_H
