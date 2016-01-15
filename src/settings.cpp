/** This file is part of Qt Media Hub**

Copyright (c) 2012 Nokia Corporation and/or its subsidiary(-ies).*
All rights reserved.

Contact:  Nokia Corporation qmh-development@qt-project.org

You may use this file under the terms of the BSD license
as follows:

Redistribution and use in source and binary forms, with or
without modification, are permitted provided that the following
conditions are met:
* Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.

* Neither the name of Nokia Corporation and its Subsidiary(-ies)
nor the names of its contributors may be used to endorse or promote
products derived from this software without specific prior
written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. **/

#include "settings.h"

#include <QtDebug>
#include <QRect>

//#define QMH_SETTINGS_DEBUG

Settings::Settings(QObject *parent) :
    QQmlPropertyMap(parent),
    m_settings(0)
{
}

bool Settings::isEnabled(const QString &name) const
{
    return value(name).toBool();
}

const QString Settings::doc(const QString &name) const
{
    return m_docs.value(name);
}

bool Settings::save()
{
    if (!m_settings)
        return false;

    if (!m_settings->isWritable()) {
        qWarning() << "settings file is not writeable" << m_settings->fileName();
        return false;
    }

    foreach (const QString &key, keys()) {
        m_settings->setValue(key, value(key));
    }

    m_settings->sync();

#ifdef QMH_SETTINGS_DEBUG
    qWarning() << "saving settings to file" << m_settings->fileName();
#endif

    return true;
}

void Settings::loadConfigFile(const QString &fileName)
{
    if (keys().isEmpty()) {
        return;
    }

    if (m_settings)
        delete m_settings;

    if (!fileName.isEmpty()) {
        m_settings = new QSettings(fileName, QSettings::IniFormat, this);
    } else {
        m_settings = new QSettings(this);
    }

    foreach (const QString &key, m_settings->allKeys()) {
        if (keys().contains(key)) {
            insert(key, checkSpecialArgumentTypes(m_settings->value(key)));
            emit valueChanged(key, value(key));
        }
    }

#ifdef QMH_SETTINGS_DEBUG
    qWarning() << "settings after parsing config file";
    print();
#endif
}

void Settings::parseArguments(const QStringList &arguments)
{
    if (keys().isEmpty()) {
        return;
    }

    // TODO use prefix to separate skin specific with global options
    foreach (const QString &key, keys()) {
        QVariant v = valueFromCommandLine(key, arguments);
        if (v.isValid()) {
            insert(key, v);
            emit valueChanged(key, v);
        }
    }

#ifdef QMH_SETTINGS_DEBUG
    qWarning() << "settings after parsing arguments";
    print();
#endif
}

void Settings::print()
{
    qWarning() << "Settings";
    foreach (const QString &key, keys()) {
        qWarning() << "\t" << key << "\t" << value(key);
    }
    if (m_settings)
        qWarning() << "settings file" << m_settings->fileName();
}

const QVariant Settings::valueFromCommandLine(const QString &key, const QStringList &arguments)
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
            value = checkSpecialArgumentTypes(arguments.value(arg + 1));
        }
    }

    return value;
}

void Settings::addOptionEntry(const QString &name, const QVariant &value, const QString &doc)
{
    const QVariant var = checkSpecialArgumentTypes(value.toString());

    m_docs.insert(name, doc);
    insert(name, var);
    emit valueChanged(name, var);
}

const QVariant Settings::checkSpecialArgumentTypes(const QString &argument)
{
    // handle rect input
    QRegExp regexp("\\d*x\\d*");
    regexp.setCaseSensitivity(Qt::CaseInsensitive);
    if (regexp.exactMatch(argument)) {
        const QStringList stringList = argument.split('x', QString::KeepEmptyParts, Qt::CaseInsensitive);
        return QRect(0, 0, stringList[0].toInt(), stringList[1].toInt());
    }

    // handle bool
    if (argument.compare("false", Qt::CaseInsensitive) == 0)
        return QVariant(false);
    if (argument.compare("true", Qt::CaseInsensitive) == 0)
        return QVariant(true);

    return argument;
}

const QVariant Settings::checkSpecialArgumentTypes(const QVariant &argument)
{
    return checkSpecialArgumentTypes(argument.toString());
}
