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

#ifndef PLAYLIST_H
#define PLAYLIST_H

#include <QtGui>
#include <QtSql>

class Playlist : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(PlayModeRoles playMode READ playMode WRITE setPlayMode NOTIFY playModeChanged)
    Q_PROPERTY(QString mediaType READ mediaType NOTIFY mediaTypeChanged)
    Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex NOTIFY currentIndexChanged)

    Q_ENUMS(PlaylistRoles)
    Q_ENUMS(DepthRoles)
    Q_ENUMS(PlayModeRoles)

public:
    enum PlaylistRoles {
        Replace,
        Append
    };

    enum DepthRoles {
        Single,
        Flat,
        Recursive
    };

    enum PlayModeRoles {
        Normal,
        Shuffle
    };

    enum CustomRole {
        PreviewUrlRole = Qt::UserRole,
        FieldRolesBegin
    };

    Playlist(QObject *parent = 0);
    void initialize();

    Q_INVOKABLE int add(const QModelIndex &index, PlaylistRoles role = Replace, DepthRoles depth = Single);
    Q_INVOKABLE int getRoleByName(const QString &roleName) const;

    Q_INVOKABLE int next();
    Q_INVOKABLE int previous();

    PlayModeRoles playMode() const;
    void setPlayMode(PlayModeRoles mode);

    QString mediaType() const;

    void setCurrentIndex(int idx);
    int currentIndex() const;

    // reimp
    Q_INVOKABLE int rowCount(const QModelIndex &parent = QModelIndex()) const;
    Q_INVOKABLE QModelIndex index(int row) const;
    Q_INVOKABLE QVariant data(int idx, int role) const;
    Q_INVOKABLE QVariant data(int idx, const QString &role) const;

    QVariant data(const QModelIndex &index, int role) const;

signals:
    void playModeChanged();
    void mediaTypeChanged();
    void currentIndexChanged();

private:
    void setMediaType(const QString &type);

    QList<QHash<int, QVariant> > m_data;
    PlayModeRoles m_playMode;
    QString m_mediaType;
    QSqlDriver *m_driver;
    int m_currentIndex;
};

#endif // PLAYLIST_H
