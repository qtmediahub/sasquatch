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
#include "media.h"

#include <QList>
#include <QMetaEnum>
#include <QModelIndex>

#define DEBUG qDebug() << __PRETTY_FUNCTION__

Playlist::Playlist(QObject *parent)
    : QAbstractListModel(parent)
    , m_playMode(Normal)
{
    QHash<int, QByteArray> roleNames = QAbstractListModel::roleNames();
    roleNames.unite(Media::roleNames());
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
        const PlaylistItem &info = content.at(index.row());
        if (role == Qt::DisplayRole) {
            return info.name;
        } else if (role == Media::PreviewUrlRole) {
            return info.previewUrl;
        } else if (role == Media::FilePathRole) {
            return info.filePath;
        }
    }

    return rv;
}

int Playlist::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return content.count();
}

QModelIndex Playlist::add(const QModelIndex &index, PlaylistRoles role, DepthRoles depth)
{
    if (!index.isValid()) {
        DEBUG << "index is not valid, epic fail";
        return QModelIndex(); // we don't have a model() to work with
    }

    if (role == Playlist::Replace && rowCount() > 0) {
        beginRemoveRows(QModelIndex(), 0, rowCount()-1);
        content.clear();
        endRemoveRows();
    }

    // check if already in playlist
    int pos = -1;
    for (int i = 0; i < content.count(); ++i) {
        if (content.at(i).filePath == index.data(Media::FilePathRole).toString()) {
            pos = i;
            break;
        }
    }

    if (pos == -1) {
        if (depth == Playlist::Flat) {
            // ## this needs to support fetchmore?
            QModelIndex parent = index.parent();
            for (int i = 0; i < index.model()->rowCount(parent); i++)
                appendItem(parent.child(i, 0));
        }
        pos = index.row(); // ## incorrect if it's invalid
    }

    DEBUG << "Playlist now has " << rowCount() << " items";

    return createIndex(pos, 0);
}

QModelIndex Playlist::index(int row) const
{
    return createIndex(row, 0);
}

QModelIndex Playlist::playNextIndex(const QModelIndex &idx) const
{
    QModelIndex next;

    if (m_playMode == Shuffle) {
        next = index(int((qreal(qrand())/RAND_MAX)*rowCount()));
    } else {
        if (idx.row() >= rowCount()-1)
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
        prev = index(rowCount()-1);
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

void Playlist::appendItem(const QModelIndex  &index)
{
    int start = rowCount()-1 < 0 ? 0 : rowCount()-1;
    int end = rowCount() < 0 ? 1 : rowCount();

    emit beginInsertRows(QModelIndex(), start, end);
    PlaylistItem item;
    item.name = index.data(Qt::DisplayRole).toString();
    item.filePath = index.data(Media::FilePathRole).toString();
    item.previewUrl = index.data(Media::PreviewUrlRole).toString();
    content.append(item);
    emit endInsertRows();
}

