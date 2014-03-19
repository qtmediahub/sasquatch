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

#if defined(QT5) && defined(QT_WIDGETS)
#include <QGraphicsView>
#endif

#ifdef QT5
#include <QGuiApplication>
#else
#include <QApplication>
#endif

#include <QKeyEvent>

#include "actionmapper.h"
#include "libraryinfo.h"
#include "globalsettings.h"

ActionMapper::ActionMapper(GlobalSettings *settings, QObject *parent) :
    QObject(parent),
    m_generatedEvent(false),
    m_skipGeneratedEvent(false),
    m_settings(settings)
{
    setupQtKeyMap();

    m_mapName = m_settings->value(GlobalSettings::Keymap).toString();

    qDebug() << "Available maps " << availableMaps();

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

void ActionMapper::processKey(qlonglong key)
{
    if (m_recipient.isNull()) {
        qWarning("Trying to send a key when no recipient is set");
        return;
    }

    int code = (int)key;
    bool upper = false;

    // lower-/upper-case characters
    if (key >= 97 && key <=122) {
        code -= 32;
    } else if (key >= Qt::Key_A && key <= Qt::Key_Z) {
        upper = true;
    }

    QKeyEvent keyPress(QEvent::KeyPress, code, upper ? Qt::ShiftModifier : Qt::NoModifier, QString(int(key)));
    qApp->sendEvent(m_recipient.data(), &keyPress);
    QKeyEvent keyRelease(QEvent::KeyRelease, code, upper ? Qt::ShiftModifier : Qt::NoModifier, QString(int(key)));
    qApp->sendEvent(m_recipient.data(), &keyRelease);
}

void ActionMapper::populateMap()
{
    m_actionMap.clear();
    foreach (const QString &keyboardMapPath, LibraryInfo::keyboardMapPaths(m_settings)) {
        QString keymap = keyboardMapPath + QString::fromLatin1("/") + m_mapName;
        qDebug() << "Try to load keymap" << m_mapName << "from keymap path" << keyboardMapPath;
        if (loadMapFromDisk(keymap)) {
            qDebug() << "Using keymap" << keymap;
            break;
        }
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

        int index = actionEnum.keyToValue(mapping[0].toLatin1().constData());
        if (index == -1) {
            qWarning() << "\tMapped action is not defined in ActionMapper, skipping: " << mapping[0];
            continue;
        }

        foreach(const QString &key, keyStrings) {
            int keyIndex = keyEnum.keyToValue(QString("Key_").append(key).toLatin1().constData());
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
    foreach (const QString &keyboardMapPath, LibraryInfo::keyboardMapPaths(m_settings)) {
        foreach (const QString &keymap, QDir(keyboardMapPath).entryList(QDir::Files)) {
            if (maps.contains(keymap)) {
                continue;
            }
            maps << keymap;
        }
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
    m_skipGeneratedEvent = false;
    recipient->installEventFilter(this);
    QObject *potentialView = 0;
#if QT_WIDGETS
    potentialView = qobject_cast<QGraphicsView*>(recipient);
#endif
    if (potentialView) {
        // directly send to the scene, to avoid loops
#if QT_WIDGETS
        m_recipient = QWeakPointer<QObject>(qobject_cast<QGraphicsView*>(potentialView)->scene());
#endif
    } else {

        //feeding outselves: spare our children!
        m_skipGeneratedEvent = true;
        m_recipient = QPointer<QObject>(recipient);
    }
}

bool ActionMapper::eventFilter(QObject *obj, QEvent *event)
{
    if (!(m_skipGeneratedEvent && m_generatedEvent) &&
            (event->type() == QEvent::KeyPress || event->type() == QEvent::KeyRelease)) {
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
#ifdef QT5
                QGuiApplication::postEvent(m_recipient.data(), e);
#else
                QApplication::postEvent(m_recipient.data(), e);
#endif
                m_generatedEvent = true;
                return true;
            } else {
                qDebug() << "The intended recipient has been forcibly shuffled off this mortal coil";
            }
        }
    }

    m_generatedEvent = false;
    // standard event processing
    return QObject::eventFilter(obj, event);
}

