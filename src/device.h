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

#ifndef DEVICE_H
#define DEVICE_H

#include <QObject>

#ifndef QT_NO_DBUS
class SystemHelperDBus;
#endif

class Device : public QObject
{
    Q_OBJECT
    Q_ENUMS(DeviceType)
    Q_PROPERTY(QString path READ path NOTIFY changed)
    Q_PROPERTY(DeviceType type READ type NOTIFY changed)
    Q_PROPERTY(QString label READ label NOTIFY changed)
    Q_PROPERTY(QString uuid READ uuid NOTIFY changed)

public:
    enum DeviceType { Undefined, UsbDrive, Dvd, AudioCd, TypeCount };

    Device(const QString &p, DeviceType t, const QString &l, const QString &u, QObject *parent = 0);

    QString path() const { return m_path; }
    DeviceType type() const { return m_type; }
    QString label() const { return m_label; }
    QString uuid() const { return m_uuid; }

signals:
    void changed();

public slots:
    void mount();
    void unmount();
    void eject();

private:
    QString m_path;
    DeviceType m_type;
    QString m_label;
    QString m_uuid;

#ifndef QT_NO_DBUS
    SystemHelperDBus *m_helper;
#endif
};

#endif // DEVICE_H
