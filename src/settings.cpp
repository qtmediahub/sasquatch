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

struct OptionEntry { Settings::Option e; QVariant value; QString name; QString doc; };

static struct OptionEntry table[] = {
    { Settings::Skin, "confluence", "skin", "specifies the skin" },
    { Settings::OverlayMode, "false", "overlay-mode", "toggle overlay mode used for devices with other mediaplayers than QtMultimediaKit" }
};

Settings::Settings(const QStringList &arguments, QObject *parent)
    : QObject(parent),
      m_arguments(arguments)
{
    load();
    parseArguments();
}

Settings::~Settings()
{
}

QVariant Settings::value(Settings::Option option)
{
    return table[option].value;
}

void Settings::setValue(Settings::Option option, QVariant value)
{
    table[option].value = value;
}

bool Settings::save()
{
    if (!m_settings.isWritable()) {
        qWarning() << "settings file is not writeable" << m_settings.fileName();
        return false;
    }

    m_settings.beginGroup("General");

    for (int i = 0; i < OptionLength; ++i) {
        m_settings.setValue(table[i].name, table[i].value);
    }

    m_settings.endGroup();
    m_settings.sync();

    return true;
}

void Settings::load()
{
    m_settings.beginGroup("General");

    foreach (QString key, m_settings.allKeys()) {
        for (int i = 0; i < OptionLength; ++i) {
            if (key == table[i].name) {
                table[i].value = m_settings.value(key, table[i].value);
                break;
            }
        }
    }

    m_settings.endGroup();
}

void Settings::parseArguments()
{
    for (int i = 0; i < OptionLength; ++i) {
        QVariant v = valueFromCommandLine(table[i].name);
        if (v.isValid()) {
            table[i].value = v;
        }
    }
}

QVariant Settings::valueFromCommandLine(const QString &key)
{
    QRegExp rx(QString("--?%1=(.*)").arg(key));
    rx.setCaseSensitivity(Qt::CaseInsensitive);
    QVariant value;
    int arg = m_arguments.indexOf(rx);
    if (arg != -1) {
        value = rx.cap(1);
    } else {
        rx.setPattern(QString("--?%1$").arg(key));
        arg = m_arguments.indexOf(rx);
        if (arg != -1 && arg + 1 < m_arguments.size()) {
            value = m_arguments.value(arg + 1);
        }
    }
    qDebug() << "found value" << key << value;
    return value;
}

