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

#include "actionmapper.h"

#include "backend.h"
#include "frontend.h"

#include "qmh-config.h"

ActionMapper::ActionMapper(QObject *p)
    : QObject(p),
      m_parent(p),
      m_mapPath(Backend::instance()->basePath() + "/devices/keymaps/")
{
    setObjectName("qmhrpc");

    m_maps = QDir(m_mapPath).entryList(QDir::Files);
    qDebug() << "Available keyboard maps" << m_maps;

    m_mapName = Config::value("keymap", "stdkeyboard").toString();
    populateMap();
}

void ActionMapper::takeAction(Action action)
{
    QHash<int, Action>::const_iterator it;
    for (it = m_actionMap.constBegin(); it != m_actionMap.constEnd(); ++it) {
        if (it.value() == action)
            break;
    }
    if (it == m_actionMap.constEnd())
        return;
    QKeyEvent keyPress(QEvent::KeyPress, it.key(), Qt::NoModifier);
    qApp->sendEvent(m_parent, &keyPress);
    QKeyEvent keyRelease(QEvent::KeyRelease, it.key(), Qt::NoModifier);
    qApp->sendEvent(m_parent, &keyRelease);
}

void ActionMapper::populateMap()
{
    m_actionMap.clear();
    loadMapFromDisk(m_mapPath + m_mapName);
}

bool ActionMapper::loadMapFromDisk(const QString &mapFilePath)
{
    const QMetaObject &ActionMO = ActionMapper::staticMetaObject;
    int enumIndex = ActionMO.indexOfEnumerator("Action");
    QMetaEnum actionEnum = ActionMO.enumerator(enumIndex);

    enumIndex = staticQtMetaObject.indexOfEnumerator("Key");
    QMetaEnum keyEnum = staticQtMetaObject.enumerator(enumIndex);

    QFile mapFile(mapFilePath);
    if (!mapFile.exists() || !mapFile.open(QIODevice::ReadOnly)) {
        qWarning() << "Could not load keymap: " << mapFilePath;
        return false;
    }

    QTextStream mapStream(&mapFile);
    while (!mapStream.atEnd()) {
        QStringList mapping = mapStream.readLine().split("=");
        QStringList keyStrings = mapping.at(1).split(",");
        QList<int> keys;

        int index = actionEnum.keyToValue(mapping[0].toAscii().constData());
        if (index == -1) {
            qWarning() << "\tMapped action is not defined in ActionMapper, skipping: " << mapping[0];
            continue;
        }

        foreach(const QString &key, keyStrings) {
            int keyIndex = keyEnum.keyToValue(QString("Key_").append(key).toAscii().constData());
            if (keyIndex == -1) {
                qWarning() << "\tQt Key does not exist: Key_" << key;
                continue;
            }
            m_actionMap[keyIndex] = static_cast<Action>(index);
        }
    }

    return true;
}

int ActionMapper::mapKeyEventToAction(QObject *event)
{
    const int key = event->property("key").toInt();
    if (m_actionMap.contains(key))
        return m_actionMap[key];
    return Null;
}

void ActionMapper::setMap(const QString &map)
{
    m_mapName = map; 
    populateMap();
}

QStringList ActionMapper::availableMaps() const
{
    return m_maps;
}

