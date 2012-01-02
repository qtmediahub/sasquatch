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

#include "skinmanager.h"
#include "libraryinfo.h"
#include "tarfileengine.h"
#include "globalsettings.h"

SkinManager::SkinManager(GlobalSettings *settings, QObject *parent) :
    QObject(parent),
    m_settings(settings)
{
    connect(&m_pathMonitor, SIGNAL(directoryChanged(QString)), this, SLOT(handleDirChanged(QString)));
    foreach (const QString &skinPath, LibraryInfo::skinPaths(m_settings)) {
        if (QDir(skinPath).exists())
            m_pathMonitor.addPath(skinPath);
    }

    m_tarFileEngineHandler = new TarFileEngineHandler;
    discoverSkins();
}

SkinManager::~SkinManager()
{
    delete m_tarFileEngineHandler;
}

QHash<QString, Skin *> SkinManager::skins() const
{
    return m_skins;
}

void SkinManager::handleDirChanged(const QString &dir)
{
    if (LibraryInfo::skinPaths(m_settings).contains(dir)) {
        qWarning() << "Changes in skin path, repopulating skins";
        discoverSkins();
    }
}

void SkinManager::discoverSkins()
{
    qDeleteAll(m_skins.values()); // FIXME: err, skin pointer is help by run-time, this will cause a crash
    m_skins.clear();

    foreach (const QString &skinPath, LibraryInfo::skinPaths(m_settings)) {
        QStringList potentialm_skins = QDir(skinPath).entryList(QDir::Dirs | QDir::NoDotAndDotDot);

        foreach(const QString &currentPath, potentialm_skins) {
            const QString prospectivePath = skinPath % "/" % currentPath;
            if (Skin *skin = Skin::createSkin(prospectivePath, this))
                m_skins.insert(skin->name(), skin);
        }
    }

    if (m_skins.isEmpty()) {
        qWarning() << "No skins are found in your skin paths:";
        foreach (const QString &skinPath, LibraryInfo::skinPaths(m_settings)) {
            qWarning() << skinPath;
        }
        qWarning() << "Please specify the '-skinsPath <path>' startup argument.";
    } else {
        QStringList sl;
        foreach(Skin *skin, m_skins)
            sl.append(skin->name());
        qDebug() << "Available skins:" << sl.join(",");
    }
}

QList<QObject *> SkinManager::skinsModel() const
{
    QList<QObject *> list;
    foreach(QObject *skin, m_skins.values())
        list << skin;
    return list;
}
