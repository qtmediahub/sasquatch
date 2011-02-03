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

Playlist::Playlist(QObject *parent)
    : QAbstractListModel(parent)
    , m_playMode(Normal)
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
    roleNames[MediaInfoRole] = "mediaInfo";
    setRoleNames(roleNames);
}

Playlist::~Playlist()
{
}

QVariant Playlist::data(const QModelIndex &index, int role) const
{
    QVariant rv;

    if (!index.isValid())
        return rv;

    if (0 <= index.row() && index.row() < content.size()) {
        MediaInfo *info = content.at(index.row());
        if (role == Qt::DisplayRole || role == FileNameRole)
            return info->name;
        else if (role == PreviewUrlRole) {
            int idx = MediaModel::staticMetaObject.indexOfEnumerator("MediaType");
            QMetaEnum e = MediaModel::staticMetaObject.enumerator(idx);
            QString urlBase = "image://" + QString::fromLatin1(e.valueToKey(info->mediaType)).toLower() + "model";
            return QUrl(urlBase + info->thumbnailPath);
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
        } else if (role == MediaInfoRole) {
            return qVariantFromValue(info);
        }
    }

    return rv;
}

int Playlist::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return content.count();
}

void Playlist::addSubTree(MediaInfo *info)
{
    sort(info);

    foreach (MediaInfo *i, info->children) {
        if (i->type == MediaModel::Directory)
            addSubTree(i);
        else if (i->type == MediaModel::File)
            append(copyMediaInfo(i));
    }
}

QModelIndex Playlist::add(MediaInfo *info, PlaylistRoles role, DepthRoles depth)
{
    if (!info)
        return QModelIndex();

#ifdef PLAYLIST_DEBUG
    qDebug() << "Playlist add" << info->filePath << "width role" << role << "and depth" << depth;
#endif

    if (role == Playlist::Replace && count() > 0) {
        beginRemoveRows(QModelIndex(), 0, count()-1);
        qDeleteAll(content);
        content.clear();
        endRemoveRows();
    }

    // check if already in playlist
    int pos = -1;
    for (int i = 0; i < content.count(); ++i) {
        if (content.at(i)->hash == info->hash) {
            pos = i;
            break;
        }
    }

    if (pos == -1) {
        if (depth == Playlist::Single) {
            if (info->type == MediaModel::File)
                append(copyMediaInfo(info));
            else
                addSubTree(info);
        } else if (depth == Playlist::Flat) {
            if (info->parent) {
                sort(info);
                foreach (MediaInfo *i, info->parent->children) {
                    if (i->type == MediaModel::File)
                        append(copyMediaInfo(i));
                }
            } else
                append(copyMediaInfo(info));
        } else {
            if (info->type == MediaModel::Directory || info->type == MediaModel::SearchPath)
                addSubTree(info);
            else
                append(copyMediaInfo(info));
        }
        for (int i = 0; i < content.count(); ++i) {
            if (content.at(i)->hash == info->hash) {
                pos = i;
                break;
            }
        }
    }

#ifdef PLAYLIST_DEBUG
    dump();
#endif

    // if we added a folder and it was not empty, return the first item
    if (info->type == MediaModel::Directory && content.count() > 0)
        pos = 0;

    return index(pos);
}

void Playlist::dump() const
{
    qDebug() << "playlist:" << content.count() << "elements";
    for (int i = 0; i < content.count(); i++) {
        qDebug() << "  " << content[i]->filePath;
    }
}

QModelIndex Playlist::index(int row ) const
{
    return QAbstractListModel::index(row, 0, QModelIndex());
}

QModelIndex Playlist::indexFromMediaInfo(MediaInfo *info) const
{
    if (!info)
        return QModelIndex();

    return index(content.indexOf(info));
}

QModelIndex Playlist::playNextIndex(const QModelIndex &idx) const
{
    QModelIndex next;

    if (m_playMode == Shuffle) {
        next = index(int((qreal(qrand())/RAND_MAX)*count()));
    } else {
        if (idx.row() >= count()-1)
            next = index(0);
        else
            next = index(idx.row()+1);
    }

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

void Playlist::setPlayMode(Playlist::PlayModeRoles mode)
{
    if (m_playMode != mode) {
        m_playMode = mode;
        emit playModeChanged();
    }
}

void Playlist::append(MediaInfo *info)
{
    int start = count()-1 < 0 ? 0 : count()-1;
    int end = count() < 0 ? 1 : count();

    emit beginInsertRows(QModelIndex(), start, end);
    content.append(info);
    emit endInsertRows();
}

bool playlistNameLessThan(MediaInfo *info1, MediaInfo *info2)
{
    return QString::localeAwareCompare(info1->name.toLower(), info2->name.toLower()) < 0;
}

void Playlist::sort(MediaInfo *info)
{
    emit layoutAboutToBeChanged();
    qSort(info->children.begin(), info->children.end(), playlistNameLessThan); // ## do this recursively for every Directory?
    emit layoutChanged();
}

int Playlist::row(const QModelIndex &idx) const
{
    return idx.row();
}

MediaInfo * Playlist::copyMediaInfo(MediaInfo *info)
{
    MediaInfo *i = new MediaInfo(info->type, info->filePath, info->mediaType);

    i->hash = info->hash;
    i->name = info->name;
    i->fileSize = info->fileSize;
    i->fileDateTime = info->fileDateTime;
    i->thumbnailPath = info->thumbnailPath;

    return i;
}



