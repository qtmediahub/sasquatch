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

#include "device.h"

Device::Device(const QString &p, QObject *parent) :
    QObject(parent)
  , m_path(p)
  , m_type(Device::Undefined)
{
#ifndef QT_NO_DBUS
    m_deviceInterface = new UDisksDeviceInterface("org.freedesktop.UDisks", m_path, QDBusConnection::systemBus(), this);
    if (!m_deviceInterface->isValid()) {
        m_valid = false;
        return;
    }
    m_valid = true;
    m_isPartition = m_deviceInterface->DeviceIsPartition();
    m_mountPoint = m_deviceInterface->DeviceMountPath();
    m_label = m_deviceInterface->IdLabel();
    m_uuid = m_deviceInterface->IdUuid();
    m_type = Device::UsbDrive;
    connect(m_deviceInterface, SIGNAL(Changed()), this, SLOT(deviceChanged()));

#else
    // no implementation yet, so not valid
    m_valid = false;
    m_isPartition = false;
#endif

    emit changed();
}

void Device::mount()
{
#ifndef QT_NO_DBUS
    m_deviceInterface->FilesystemMount();
#endif
}

void Device::unmount()
{
#ifndef QT_NO_DBUS
    m_deviceInterface->FilesystemUnmount();
#endif
}

void Device::eject()
{
#ifndef QT_NO_DBUS
    m_deviceInterface->DriveEject();
#endif
}

void Device::deviceChanged()
{
#ifndef QT_NO_DBUS
    m_isPartition = m_deviceInterface->DeviceIsPartition();
    m_mountPoint = m_deviceInterface->DeviceMountPath();
    m_label = m_deviceInterface->IdLabel();
    m_uuid = m_deviceInterface->IdUuid();
    m_type = Device::UsbDrive;
#endif

    emit changed();
}
