/****************************************************************************

This file is part of the QtMediaHub project on http://www.gitorious.org.

Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).*
All rights reserved.

Contact:  Nokia Corporation (qt-info@nokia.com)**

You may use this file under the terms of the BSD license as follows:

"Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
* Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
* Neither the name of Nokia Corporation and its Subsidiary(-ies) nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."

****************************************************************************/

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
    //void mount(const QString &path);
    //void unmount(const QString &path);
    //void eject(const QString &path);

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
