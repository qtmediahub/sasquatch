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

#include "skin.h"
#include "libraryinfo.h"
#include "jsonparser/json.h"

#include <QtGui>
#include <QtDebug>

Skin::Skin(QObject *parent)
    : QObject(parent),
      m_settings(new Settings(this))
{
}

Skin::~Skin()
{
    m_settings->save();
}

QString Skin::name() const
{
    return m_name;
}

QString Skin::path() const
{
    return m_path;
}

QString Skin::config() const
{
    return m_config;
}

Skin *Skin::createSkin(const QString &skinPath, QObject *parent)
{
    QString mapFile = skinPath + "/skin.manifest";
    if (!QFile(mapFile).exists())
        return 0;

    Skin *skin = new Skin(parent);
    QFileInfo fileInfo(skinPath);
    skin->m_name = fileInfo.fileName();
    skin->m_path = skinPath; // fileInfo.canonicalFilePath() doesn't work for qar files
    skin->m_config = mapFile;

    return skin;
}

QUrl Skin::urlForResolution(const QString &nativeResolutionString, const QString &fallbackResolution)
{
    //http://en.wikipedia.org/wiki/720p
    //1440, 1080, 720, 576, 480, 360, 240
    QHash<QString, QString> resolutionHash;
    resolutionHash["1440p"] = "2560x1440";
    resolutionHash["1080p"] = "1920x1080";
    resolutionHash["720p"] = "1280x720";

    QFile skinConfig(m_config);
    if (!skinConfig.open(QIODevice::ReadOnly)) {
        qWarning() << "Can't read " << m_config << " of skin " << m_name;
        return QUrl();
    }

    JsonReader reader;
    if (!reader.parse(skinConfig.readAll())) {
        qWarning() << "Failed to parse config file " << m_config << reader.errorString();
        return QUrl();
    }
    QHash<QString, QString> resolutionToFile;

    const QVariantMap root = reader.result().toMap();
    const QVariantMap resolutions = root["resolutions"].toMap();
    foreach (const QVariant &v, resolutions) {
        QString name = v.toString();
        QString resolutionSize = resolutionHash.contains(name) ? resolutionHash[name] : name;
        QVariantMap resolution = v.toMap();
        resolutionToFile[resolutionSize] = resolutions[name].toMap()["file"].toString();
    }
    resolutionToFile["default"] = resolutions[root["default_resolution"].toString()].toMap()["file"].toString();

    // load default settings
    const QVariantList settings = root["settings"].toList();
    foreach (const QVariant &s, settings) {
        const QVariantMap entry = s.toMap();
        m_settings->addOptionEntry(entry.value("name").toString(), entry.value("default_value").toString(), entry.value("doc").toString());
    }
    const QString configFilePath = QFileInfo(QSettings().fileName()).absolutePath() + QLatin1String("/") + name() + QLatin1String(".ini");
    m_settings->loadConfigFile(configFilePath);
    m_settings->parseArguments(QApplication::arguments(), name());

    QString urlPath =
            resolutionToFile.contains(nativeResolutionString)
            ? resolutionToFile[nativeResolutionString]
            : resolutionToFile[fallbackResolution];

    return QUrl::fromLocalFile(m_path % "/" % urlPath);
}

bool Skin::isRemoteControl() const
{
    return m_name.contains("remote"); // ## obviously bad
}

Skin::Type Skin::type(const QUrl &url) const
{
    if (url.path().right(3) == "qml") {
        return Qml;
    }
    return Invalid;
}

Settings *Skin::settings() const
{
    return m_settings;
}

