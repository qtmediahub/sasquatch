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

#include "actionmapper.h"
#include "libraryinfo.h"

#ifdef SCENEGRAPH
#include <QGraphicsView>
#include <QApplication>
#endif

#include <QKeyEvent>

#include "qmh-config.h"

ActionMapper::ActionMapper(QObject *parent)
    : QObject(parent)
{
    setupQtKeyMap();

    qDebug() << "Available maps " << availableMaps();
    m_mapName = Config::value("keymap", "stdkeyboard").toString();
    qDebug() << "used keymap" << m_mapName;
    populateMap();
}

QHash<ActionMapper::Action, Qt::Key> ActionMapper::s_actionToQtKeyMap;

void ActionMapper::setupQtKeyMap()
{
    static bool initialized = false;
    if (initialized)
        return;
    s_actionToQtKeyMap.insert(ActionMapper::Left, Qt::Key_Left);
    s_actionToQtKeyMap.insert(ActionMapper::Right, Qt::Key_Right);
    s_actionToQtKeyMap.insert(ActionMapper::Up, Qt::Key_Up);
    s_actionToQtKeyMap.insert(ActionMapper::Down, Qt::Key_Down);
    s_actionToQtKeyMap.insert(ActionMapper::Enter, Qt::Key_Enter);
    s_actionToQtKeyMap.insert(ActionMapper::Menu, Qt::Key_Menu);
    s_actionToQtKeyMap.insert(ActionMapper::Context, Qt::Key_Context1);
    s_actionToQtKeyMap.insert(ActionMapper::ContextualUp, Qt::Key_PageUp);
    s_actionToQtKeyMap.insert(ActionMapper::ContextualDown, Qt::Key_PageDown);
    s_actionToQtKeyMap.insert(ActionMapper::MediaPlayPause, Qt::Key_MediaTogglePlayPause);
    s_actionToQtKeyMap.insert(ActionMapper::MediaStop, Qt::Key_MediaStop);
    s_actionToQtKeyMap.insert(ActionMapper::MediaPrevious, Qt::Key_MediaPrevious);
    s_actionToQtKeyMap.insert(ActionMapper::MediaNext, Qt::Key_MediaNext);
    s_actionToQtKeyMap.insert(ActionMapper::Back, Qt::Key_Back);
    s_actionToQtKeyMap.insert(ActionMapper::VolumeUp, Qt::Key_VolumeUp);
    s_actionToQtKeyMap.insert(ActionMapper::VolumeDown, Qt::Key_VolumeDown);

    initialized = true;
}

void ActionMapper::takeAction(Action action)
{
    if (m_recipient.isNull()) {
        qWarning("Trying to send an action when no recipient is set");
        return;
    }
    Qt::Key key = s_actionToQtKeyMap.value(action);
    QKeyEvent keyPress(QEvent::KeyPress, key, Qt::NoModifier);
    qApp->sendEvent(m_recipient.data(), &keyPress);
    QKeyEvent keyRelease(QEvent::KeyRelease, key, Qt::NoModifier);
    qApp->sendEvent(m_recipient.data(), &keyRelease);
}

void ActionMapper::processKey(int key)
{
    if (m_recipient.isNull()) {
        qWarning("Trying to send a key when no recipient is set");
        return;
    }

    int code = key;
    bool upper = false;

    // lower-/upper-case characters
    if (key >= 97 && key <=122) {
        code -= 32;
    } else if (key >= Qt::Key_A && key <= Qt::Key_Z) {
        upper = true;
    }

    QKeyEvent keyPress(QEvent::KeyPress, code, upper ? Qt::ShiftModifier : Qt::NoModifier, QString(key));
    qApp->sendEvent(m_recipient.data(), &keyPress);
    QKeyEvent keyRelease(QEvent::KeyRelease, code, upper ? Qt::ShiftModifier : Qt::NoModifier, QString(key));
    qApp->sendEvent(m_recipient.data(), &keyRelease);
}

void ActionMapper::populateMap()
{
    m_actionMap.clear();
    foreach (const QString &keyboardMapPath, LibraryInfo::keyboardMapPaths()) {
        if (QDir(keyboardMapPath + m_mapName).exists())
            loadMapFromDisk(keyboardMapPath + m_mapName);
    }
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

QStringList ActionMapper::availableMaps() const
{
    QStringList maps;
    foreach (const QString &keyboardMapPath, LibraryInfo::keyboardMapPaths()) {
        maps << QDir(keyboardMapPath).entryList(QDir::Files);
    }
    return maps;
}

void ActionMapper::setMap(const QString &map)
{
    m_mapName = map; 
    populateMap();
}

void ActionMapper::setRecipient(QObject *recipient)
{
    recipient->installEventFilter(this);
    QGraphicsView *potentialView = qobject_cast<QGraphicsView*>(recipient);
    if (potentialView) {
        // directly send to the scene, to avoid loops
        m_recipient = QWeakPointer<QObject>(potentialView->scene());
    } else {
        m_recipient = QWeakPointer<QObject>(recipient);
    }
}

bool ActionMapper::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress || event->type() == QEvent::KeyRelease) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        int key = keyEvent->key();
        if (m_actionMap.contains(key)) {
            QKeyEvent *e = new QKeyEvent(keyEvent->type()
                        , s_actionToQtKeyMap.value(m_actionMap.value(keyEvent->key()))
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

