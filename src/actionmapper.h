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

#ifndef ACTIONMAPPER_H
#define ACTIONMAPPER_H

#include <QObject>
#include <QtDeclarative>
#include <QDebug>
#include <QWeakPointer>

class ActionMapper : public QObject
{
    Q_OBJECT
    Q_ENUMS(Action)
    Q_PROPERTY(QString map READ map WRITE setMap)
public:
    enum Action {
        Null = -1,
        Left,
        Up,
        Right,
        Down,
        Enter,
        Menu,
        Context,
        ContextualUp,
        ContextualDown,
        MediaPlayPause,
        MediaStop,
        MediaPrevious,
        MediaNext,
        Back,
        VolumeUp,
        VolumeDown
    };

    ActionMapper(QObject *p, QString mapPath);

    Q_INVOKABLE QStringList availableMaps() const {
        return m_maps;
    }
    QString map() const { return m_mapName; }
    void setMap(const QString &map);

    void setRecipient(QObject *recipient);

public slots:
    void takeAction(int action) { takeAction(static_cast<Action>(action)); }
    void takeAction(Action action);

protected:
    bool eventFilter(QObject *obj, QEvent *event);

private:
    bool loadMapFromDisk(const QString &mapFilePath);
    void populateMap();
    void setupInternalMap();

    QWeakPointer<QObject> m_recipient;
    QObject *m_parent;
    QString m_mapName;
    QString m_mapPath;
    QStringList m_maps;
    QHash<int, Action> m_actionMap;
    QHash<Action, Qt::Key> m_internalActionMap;
    QHash<int,int> repeatingKeys;
};

QML_DECLARE_TYPE(ActionMapper)

#endif
