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

enum MediaInfoType { Picture, Video, Music, Directory, File, DotDot, SearchPath };

class MediaInfo : public QObject
{
    Q_OBJECT
    Q_ENUMS(Status)

public:
    enum Status { NotSearched, Searching, Searched };

    MediaInfo(MediaInfoType type, const QString &path) { }
    ~MediaInfo() { }

    MediaInfo *parent;
    MediaInfoType type;
    QString hash;
    QString filePath;
    QString name;
    Status status;
    QList<MediaInfo *> children;
    qint64 fileSize;
    QDateTime fileDateTime;
    QString thumbnailPath;
};

Q_DECLARE_METATYPE(MediaInfo*)

class Playlist : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(PlayModeRoles playMode READ playMode WRITE setPlayMode NOTIFY playModeChanged)
    Q_ENUMS(CustomRoles)
    Q_ENUMS(PlaylistRoles)
    Q_ENUMS(DepthRoles)
    Q_ENUMS(PlayModeRoles)

public:
    Playlist(QObject *parent = 0);
    ~Playlist();

    enum CustomRoles {
        // Qt::UserRole+1 to 100 are reserved by this model!
        PreviewUrlRole = Qt::UserRole + 1,
        FilePathRole,
        FileNameRole,
        FileUrlRole,
        MediaInfoTypeRole,
        FileSizeRole,
        FileDateTimeRole,
        MediaInfoRole
    };

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

    int rowCount(const QModelIndex &parent) const;
    int count() const { return rowCount(QModelIndex()); }
    Q_INVOKABLE QModelIndex index ( int row ) const;
    Q_INVOKABLE int row(const QModelIndex &idx) const;
    Q_INVOKABLE QVariant data(const QModelIndex &index, int role) const;

    void dump() const;
    QString typeString() const;

    void addSubTree(MediaInfo *info);
    Q_INVOKABLE QModelIndex add(MediaInfo *info, PlaylistRoles role = Replace, DepthRoles depth = Single);

    Q_INVOKABLE QModelIndex indexFromMediaInfo(MediaInfo *info) const;
    Q_INVOKABLE QModelIndex playNextIndex(const QModelIndex &idx) const;
    Q_INVOKABLE QModelIndex playPreviousIndex(const QModelIndex &idx) const;

    PlayModeRoles playMode() { return m_playMode; }

public slots:
    void setPlayMode(PlayModeRoles mode);

signals:
    void countChanged();
    void playModeChanged();

private:
    void append(MediaInfo *info);
    void sort(MediaInfo *info);
    MediaInfo *copyMediaInfo(MediaInfo *info);

    Q_DISABLE_COPY(Playlist)
    QList<MediaInfo*> content;
    PlayModeRoles m_playMode;
};

#endif // PLAYLIST_H
