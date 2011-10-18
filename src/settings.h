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

#ifndef SETTINGS_H
#define SETTINGS_H

#include <QObject>
#include <QVariant>
#include <QSettings>
#include <QStringList>
#include <QDeclarativePropertyMap>

#include "global.h"


// TODO make it threadsafe

class QMH_EXPORT Settings : public QDeclarativePropertyMap
{
    Q_OBJECT
    Q_ENUMS(Option)

public:
    enum Option {
        Skin = 0,
        SkinsPath,
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

        OptionLength
    };

    // run Settings::instance()->init() before any other access to Settings
    void init(const QStringList &arguments);

    static Settings* instance();

    Q_INVOKABLE static bool isEnabled(Settings::Option option);
    Q_INVOKABLE static QVariant value(Settings::Option option);

    Q_INVOKABLE const QString name(Settings::Option option) const;
    Q_INVOKABLE const QString doc(Settings::Option option) const;

    Q_INVOKABLE void setValue(Settings::Option option, const QVariant &value);

    Q_INVOKABLE bool save();


private:
    Settings(QObject *parent = 0)
        : QDeclarativePropertyMap(parent)
    {
    }

    void load();
    void parseArguments(const QStringList &arguments);
    QVariant valueFromCommandLine(const QString &key, const QStringList &arguments);
    void setOptionEntry(Settings::Option option, const QVariant &value, const QString &name, const QString &doc);

    struct OptionEntry { Settings::Option option; QVariant value; QString name; QString doc; };

    QSettings m_settings;
    struct OptionEntry m_table[Settings::OptionLength];
    static Settings *m_instance;
};

#endif // SETTINGS_H
