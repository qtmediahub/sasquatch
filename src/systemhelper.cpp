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


