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

#include "skin.h"

#include <QtGui>
#include <QtDebug>

Skin::Skin(QObject *parent)
    : QObject(parent)
{
}

Skin::~Skin()
{
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
    QString mapFile = skinPath + "/skin.map";
    if (QFile(mapFile).exists()) {
        Skin *skin = new Skin(parent);
        QFileInfo fileInfo(mapFile);
        skin->m_name = fileInfo.dir().dirName();
        skin->m_path = fileInfo.absoluteDir().absolutePath();
        skin->m_config = mapFile;
        return skin;
    }

    return 0;
}

QUrl Skin::urlForResolution(const QString &nativeResolutionString, const QString &fallbackResolution)
{
    //http://en.wikipedia.org/wiki/720p
    //1440, 1080, 720, 576, 480, 360, 240
    QHash<QString, QString> resolutionHash;
    resolutionHash["1440"] = "2560x1440";
    resolutionHash["1080"] = "1920x1080";
    resolutionHash["720"] = "1280x720";

    QFile skinConfig(m_config);
    if (!skinConfig.open(QIODevice::ReadOnly)) {
        qWarning() << "Can't read " << m_config << " of skin " << m_name;
        return QUrl();
    }

    QHash<QString, QString> fileForResolution;
    QTextStream skinStream(&skinConfig);
    while (!skinStream.atEnd()) {
        QStringList resolutionToFile = skinStream.readLine().split(":");
        if (resolutionToFile.count() == 2) {
            QString resolution =
                    resolutionHash.contains(resolutionToFile.at(0))
                    ? resolutionHash[resolutionToFile.at(0)]
                    : resolutionToFile.at(0);
            fileForResolution[resolution] = resolutionToFile.at(1);
        } else {
            qWarning() << "bad line in skin configuration";
        }
    }

    QString urlPath =
            fileForResolution.contains(nativeResolutionString)
            ? fileForResolution[nativeResolutionString]
            : fileForResolution[fallbackResolution];

    return QUrl::fromLocalFile(m_path % "/" % urlPath);
}

bool Skin::isRemoteControl() const
{
    return m_name.contains("remote"); // ## obviously bad
}

