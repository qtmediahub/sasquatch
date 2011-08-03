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

#include <QKeyEvent>

#include "frontend.h"

#include "qmh-config.h"

ActionMapper::ActionMapper(QObject *p, QString mapPath)
    : QObject(p),
      m_parent(p),
      m_mapPath(mapPath + "/devices/keymaps/")
{
    setObjectName("qmhrpc");

    setupInternalMap();

    m_maps = QDir(m_mapPath).entryList(QDir::Files);
    qDebug() << "Available keyboard maps" << m_maps;

    m_mapName = Config::value("keymap", "stdkeyboard").toString();
    qDebug() << "used keymap" << m_mapName;
    populateMap();
}

void ActionMapper::takeAction(Action action)
{
    if(m_recipient.isNull()) {
        qWarning("Trying to send an action when no recipient is set");
        return;
    }
    QHash<int, Action>::const_iterator it;
    for (it = m_actionMap.constBegin(); it != m_actionMap.constEnd(); ++it) {
        if (it.value() == action)
            break;
    }
    if (it == m_actionMap.constEnd())
        return;
    QKeyEvent keyPress(QEvent::KeyPress, it.key(), Qt::NoModifier);
    qApp->sendEvent(m_recipient.data(), &keyPress);
    QKeyEvent keyRelease(QEvent::KeyRelease, it.key(), Qt::NoModifier);
    qApp->sendEvent(m_recipient.data(), &keyRelease);
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

void ActionMapper::setMap(const QString &map)
{
    m_mapName = map; 
    populateMap();
}

void ActionMapper::setRecipient(QObject *recipient) {
    recipient->installEventFilter(this);
    QGraphicsView *potentialView = qobject_cast<QGraphicsView*>(recipient);
    if (potentialView) {
        // directly send to the scene, to avoid loops
        m_recipient = QSharedPointer<QObject>(potentialView->scene());
    } else {
        m_recipient = QSharedPointer<QObject>(recipient);
    }
}

bool ActionMapper::eventFilter(QObject *obj, QEvent *event)
{
    static int keyDiscardRate = qMax(1, Config::value("keyDiscardRate", 15));
    static int primitiveKeyCompression = Config::value("keyCompress", true);

    if (event->type() == QEvent::KeyPress || event->type() == QEvent::KeyRelease) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        int key = keyEvent->key();
        if (m_actionMap.contains(key))
        {
            //squash key events
            if (primitiveKeyCompression
                    && keyEvent->isAutoRepeat())
            {
                if (repeatingKeys.contains(key))
                {
                    repeatingKeys[key] += 1;
                    if (repeatingKeys[key]%keyDiscardRate > 1) {
                        return true;
                    }
                } else {
                    repeatingKeys[key] = 0;
                }
            } else {
                repeatingKeys.remove(key);
            }
            //end squash
            QKeyEvent *e = new QKeyEvent(keyEvent->type()
                        , m_internalActionMap.value(m_actionMap.value(keyEvent->key()))
                        , keyEvent->modifiers()
                        , keyEvent->text()
                        , keyEvent->isAutoRepeat()
                        , keyEvent->count());
            if (!m_recipient.isNull()) {
                QApplication::postEvent(m_recipient.data(), e);
                return true;
            } else {
                qDebug() << "The intended recipient has been forcibly shuffled off this mortal coil";
            }
        }
    }

    // standard event processing
    return QObject::eventFilter(obj, event);
}

void ActionMapper::setupInternalMap()
{
    m_internalActionMap.insert(Left, Qt::Key_Left);
    m_internalActionMap.insert(Right, Qt::Key_Right);
    m_internalActionMap.insert(Up, Qt::Key_Up);
    m_internalActionMap.insert(Down, Qt::Key_Down);
    m_internalActionMap.insert(Enter, Qt::Key_Enter);
    m_internalActionMap.insert(Menu, Qt::Key_Menu);
    m_internalActionMap.insert(Context, Qt::Key_Context1);
    m_internalActionMap.insert(ContextualUp, Qt::Key_PageUp);
    m_internalActionMap.insert(ContextualDown, Qt::Key_PageDown);
    m_internalActionMap.insert(MediaPlayPause, Qt::Key_MediaTogglePlayPause);
    m_internalActionMap.insert(MediaStop, Qt::Key_MediaStop);
    m_internalActionMap.insert(MediaPrevious, Qt::Key_MediaPrevious);
    m_internalActionMap.insert(MediaNext, Qt::Key_MediaNext);
    m_internalActionMap.insert(Back, Qt::Key_Back);
    m_internalActionMap.insert(VolumeUp, Qt::Key_VolumeUp);
    m_internalActionMap.insert(VolumeDown, Qt::Key_VolumeDown);
}

