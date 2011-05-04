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

