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

class Frontend;

ActionMapper::ActionMapper(Frontend *p)
    : QObject(p),
      pFrontend(p),
      mapPath(Backend::instance()->basePath() + "/devices/keymaps/")
{
    setObjectName("qmhrpc");

    maps = QDir(mapPath).entryList(QDir::Files);
    qWarning() << "Available keyboard maps" << maps;

    mapName = Config::value("keymap", "").toString();
    populateMap();
}

void ActionMapper::takeAction(Action action)
{
    if (!pFrontend || !keyHash.contains(action))
        return;
    QKeyEvent keyPress(QEvent::KeyPress, keyHash[action].at(0), Qt::NoModifier);
    qApp->sendEvent(pFrontend->centralWidget(), &keyPress);
    QKeyEvent keyRelease(QEvent::KeyRelease, keyHash[action].at(0), Qt::NoModifier);
    qApp->sendEvent(pFrontend->centralWidget(), &keyRelease);
}

void ActionMapper::populateMap()
{
    keyHash.clear();

    if (!loadMapFromDisk(mapPath + mapName))
        loadMapFromDisk(mapPath + "stdkeyboard");
}

bool ActionMapper::loadMapFromDisk(const QString &mapFilePath)
{
    const QMetaObject &ActionMO = ActionMapper::staticMetaObject;
    int enumIndex = ActionMO.indexOfEnumerator("Action");
    QMetaEnum actionEnum = ActionMO.enumerator(enumIndex);

    enumIndex = staticQtMetaObject.indexOfEnumerator("Key");
    QMetaEnum keyEnum = staticQtMetaObject.enumerator(enumIndex);

    QFile mapFile(mapFilePath);
    if (mapFile.exists()
        && mapFile.open(QIODevice::ReadOnly))
    {
        qWarning() << "Load keymap" << mapFilePath;

        QTextStream mapStream(&mapFile);
        while(!mapStream.atEnd())
        {
            QStringList mapping = mapStream.readLine().split("=");
            QStringList keyStrings = mapping.at(1).split(",");
            QList<int> keys;
            foreach(const QString &key, keyStrings)
            {
                int keyIndex = keyEnum.keyToValue(QString("Key_").append(key).toAscii().constData());
                if(keyIndex == -1)
                    qWarning() << "\tQt Key does not exist: Key_" << key;
                else
                    keys << keyIndex;
            }
            int index =
                actionEnum.keyToValue(mapping[0].toAscii().constData());
            if (index == -1)
                qWarning() << "\tMapped action is not defined in ActionMapper, skipping: " << mapping[0];
            else
                keyHash[static_cast<Action>(index)] = keys;
        }
        if(actionEnum.keyCount() != keyHash.size())
            qWarning("\tCertain actions have not been mapped");
        else
            qWarning("\tAll keys mapped correctly");
    } else {
        qWarning() << "Could not load keymap: " << mapFilePath;
        return false;
    }

    return true;
}
