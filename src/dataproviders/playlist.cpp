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

#include "playlist.h"

#include <QList>
#include <QMetaEnum>
#include <QModelIndex>

//#define PLAYLIST_DEBUG

class PlaylistPrivate
{
public:
    PlaylistPrivate()
    {
    }

    QList<MediaInfo*> content;
};

Playlist::Playlist(QObject *parent) :
    QAbstractListModel(parent)
{
    QHash<int, QByteArray> roleNames = QAbstractListModel::roleNames();
    roleNames[Qt::DisplayRole] = "display";
    roleNames[PreviewUrlRole] = "previewUrl";
    roleNames[FilePathRole] = "filePath";
    roleNames[FileNameRole] = "fileName";
    roleNames[FileUrlRole] = "fileUrl";
    roleNames[MediaInfoTypeRole] = "type";
    roleNames[FileSizeRole] = "fileSize";
    roleNames[FileDateTimeRole] = "fileDateTime";
    setRoleNames(roleNames);

    d = new PlaylistPrivate;
}

Playlist::~Playlist()
{
    delete d;
}

QVariant Playlist::data(const QModelIndex &index, int role) const
{
    QVariant rv;

    if (!index.isValid())
        return rv;

    if (0 <= index.row() && index.row() < d->content.size()) {
        MediaInfo *info = d->content.at(index.row());
        if (role == Qt::DisplayRole || role == FileNameRole)
            return info->name;
        else if (role == PreviewUrlRole) {
            int idx = MediaModel::staticMetaObject.indexOfEnumerator("MediaType");
            QMetaEnum e = MediaModel::staticMetaObject.enumerator(idx);
            QString urlBase = "image://" + QString::fromLatin1(e.valueToKey(info->type)).toLower() + "model";
            return QUrl(urlBase + info->thumbnail);
        } else if (role == FileUrlRole) {
            return QUrl::fromLocalFile(info->filePath);
        } else if (role == FilePathRole) {
            return info->filePath;
        } else if (role == MediaInfoTypeRole) {
            int idx = MediaModel::staticMetaObject.indexOfEnumerator("MediaInfoType");
            QMetaEnum e = MediaModel::staticMetaObject.enumerator(idx);
            return QString::fromLatin1(e.valueToKey(info->type));
        } else if (role == FileSizeRole) {
            return info->fileSize;
        } else if (role == FileDateTimeRole) {
            return info->fileDateTime;
        }
    }

    return rv;
}

int Playlist::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return d->content.count();
}

int Playlist::add(MediaInfo *info)
{
    // check for type currently only add audio/video files
    if (info->type != MediaModel::File)
        return -1;

    int pos = d->content.indexOf(info);
    if (pos == -1) {
        d->content.append(info);
        pos = d->content.count() -1;
    }

#ifdef PLAYLIST_DEBUG
    dump();
#endif

    return pos;
}

void Playlist::dump() const
{
    qDebug() << "playlist:" << d->content.count() << "elements";
    for (int i = 0; i < d->content.count(); i++) {
        qDebug() << "  " << d->content[i]->filePath;
    }
}

QModelIndex Playlist::index(int row ) const
{
    return QAbstractListModel::index(row, 0, QModelIndex());
}

QModelIndex Playlist::playNextIndex(const QModelIndex &idx) const
{
    QModelIndex next;

    if (idx.row() >= count()-1)
        next = index(0);
    else
        next = index(idx.row()+1);

    return next;
}

QModelIndex Playlist::playPreviousIndex(const QModelIndex &idx) const
{
    QModelIndex prev;

    if (idx.row() <= 0)
        prev = index(count()-1);
    else
        prev = index(idx.row()-1);

    return prev;
}


