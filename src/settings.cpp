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

#include "settings.h"

#include <QtDebug>

Settings *Settings::m_instance = 0;

void Settings::init(const QStringList &arguments)
{
// annoying but m_table[Settings::Skin] = { Settings::Skin, "confluence", "skin", "specifies the skin" }; only possible in c++0x
//                 Settings::Option,    default value,  name,               documentation
    setOptionEntry(Skin,                "confluence",   "skin",             "<name> specifies the skin");
    setOptionEntry(SkinsPath,           "",             "skins-path",       "<path> adds path to skins search paths");
    setOptionEntry(Keymap,              "stdkeyboard",  "keymap",           "<name> specifies the keymap");
    setOptionEntry(KeymapsPath,         "",             "keymaps-path",     "<path> adds path to keymaps search paths");
    setOptionEntry(ApplicationsPath,    "",             "apps-path",        "<path> adds path to skins search paths");
    setOptionEntry(FullScreen,          "true",         "fullscreen",       "<bool> toggle fullscreen");
    setOptionEntry(OverlayMode,         "false",        "overlay-mode",     "<bool> toggle overlay mode used for devices with other mediaplayers than QtMultimediaKit");
    setOptionEntry(Headless,            "false",        "headless",         "<bool> toggle running with user interface, usable for streaming server usage");
    setOptionEntry(Proxy,               "false",        "proxy",            "<bool> use a proxy for network access");
    setOptionEntry(ProxyHost,           "localhost",    "proxy-host",       "<hostname> set proxy host, only used with -proxy=true");
    setOptionEntry(ProxyPort,           "8080",         "proxy-port",       "<port> set port number for proxy usage, only used with -proxy=true");
    setOptionEntry(MultiInstance,       "false",        "multi-instance",   "<bool> allow running multiple instances");

    // first load settings from config file
    load();

    // then load arguments as they overrule config file
    parseArguments(arguments);
}

Settings* Settings::instance()
{
    if(!m_instance) {
        m_instance = new Settings();
    }
    return m_instance;
}

QVariant Settings::value(Settings::Option option)
{
    return Settings::instance()->m_table[option].value;
}

bool Settings::isEnabled(Settings::Option option)
{
    return Settings::instance()->m_table[option].value.toBool();
}

const QString Settings::name(Settings::Option option) const
{
    return m_table[option].name;
}

const QString Settings::doc(Settings::Option option) const
{
    return m_table[option].doc;
}

void Settings::setValue(Settings::Option option, const QVariant &value)
{
    m_table[option].value = value;
}

bool Settings::save()
{
    if (!m_settings.isWritable()) {
        qWarning() << "settings file is not writeable" << m_settings.fileName();
        return false;
    }

    for (int i = 0; i < OptionLength; ++i) {
        m_settings.setValue(m_table[i].name, m_table[i].value);
    }

    m_settings.sync();

    return true;
}

void Settings::load()
{
    foreach (QString key, m_settings.allKeys()) {
        for (int i = 0; i < OptionLength; ++i) {
            if (key == m_table[i].name) {
                m_table[i].value = m_settings.value(key, m_table[i].value);
                break;
            }
        }
    }
}

void Settings::parseArguments(const QStringList &arguments)
{
    for (int i = 0; i < OptionLength; ++i) {
        QVariant v = valueFromCommandLine(m_table[i].name, arguments);
        if (v.isValid()) {
            m_table[i].value = v;
        }
    }
}

QVariant Settings::valueFromCommandLine(const QString &key, const QStringList &arguments)
{
    QRegExp rx(QString("--?%1=(.*)").arg(key));
    rx.setCaseSensitivity(Qt::CaseInsensitive);
    QVariant value;
    int arg = arguments.indexOf(rx);
    if (arg != -1) {
        value = rx.cap(1);
    } else {
        rx.setPattern(QString("--?%1$").arg(key));
        arg = arguments.indexOf(rx);
        if (arg != -1 && arg + 1 < arguments.size()) {
            value = arguments.value(arg + 1);
        }
    }

    return value;
}

void Settings::setOptionEntry(Settings::Option option, const QVariant &value, const QString &name, const QString &doc)
{
    m_table[option].option = option;
    m_table[option].value = value;
    m_table[option].name = name;
    m_table[option].doc = doc;
}




