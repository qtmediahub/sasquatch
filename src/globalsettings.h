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

#ifndef GLOBALSETTINGS_H
#define GLOBALSETTINGS_H

#include "settings.h"
#include "global.h"

class QMH_EXPORT GlobalSettings : public Settings
{
    Q_OBJECT
    Q_ENUMS(Option)

public:
    enum Option {
        Skin = 0,
        SkinsPath,
        SkinResolution,
        Keymap,
        KeymapsPath,
        ApplicationsPath,
        FullScreen,
        OverlayMode,
        Headless,
        Proxy,
        ProxyHost,
        ProxyPort,
        MultiInstance,
        Mouse,
        MouseTimeout,
        DrivenFPS,
        StreamingAddress,
        StreamingPort,
        Overscan,
        OverscanGeometry,
        IdleTimeout,
        SystemTray,
        WindowGeometry,
        OpenGLUsage,
        OpenGLFormatHack,
        OpenGLViewport,
        SmoothScaling,
        Avahi,
        AvahiAdvertize,
        AvahiHideIPv6,
        AvahiHideLocal,
        ResizeDelay,
        TranslationsPath,
        ResourcesPath,
        AppsPath,
        ImportsPath,
        ThumbnailPath,
        ThumbnailSize,
        MediaRefreshInterval,

        OptionLength
    };

    // TODO remove singleton hack only used for MediaModel
    static GlobalSettings *instance();
    GlobalSettings(QObject *parent = 0);

    Q_INVOKABLE bool isEnabled(GlobalSettings::Option option) const;
    Q_INVOKABLE QVariant value(GlobalSettings::Option option) const;
    Q_INVOKABLE const QString name(GlobalSettings::Option option) const;
    Q_INVOKABLE const QString doc(GlobalSettings::Option option) const;

    Q_INVOKABLE void setValue(GlobalSettings::Option option, const QVariant &value);

    void addOptionEntry(GlobalSettings::Option option, const QVariant &value, const QString &name, const QString &doc);

private:
    QString m_table[GlobalSettings::OptionLength];
    static GlobalSettings *m_instance;
};

#endif // GLOBALSETTINGS_H
