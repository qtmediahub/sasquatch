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

#include "playlist.h"
#include "mediamodel.h"
#include "global.h"

#define DEBUG if (0) qDebug() << __PRETTY_FUNCTION__

Playlist::Playlist(QObject *parent)
    : QAbstractListModel(parent)
    , m_playMode(Normal)
    , m_currentIndex(-1)
{
}

QVariant Playlist::data(int index, const QString &role) const
{
    return m_data.value(index).value(m_nameToRole.value(role));
}

QVariant Playlist::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    return m_data.value(index.row()).value(role);
}

int Playlist::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_data.count();
}

void Playlist::clear()
{
    beginResetModel();
    m_data.clear();
    endResetModel();
    m_currentIndex = -1;
    emit currentIndexChanged();
}

void Playlist::addCurrentLevel(MediaModel *mediaModel)
{
    add(mediaModel, -1);
}

void Playlist::add(MediaModel *mediaModel, int row)
{
    if (!mediaModel) {
        DEBUG << "invalid model";
        return;
    }

    DEBUG << "add item" << mediaModel << row;

    // if new item is not same type make this playlist of new type
    if (mediaModel->mediaType() != m_mediaType)
        setMediaType(mediaModel->mediaType());

    if (mediaModel->isLeafLevel()) { 
        QModelIndex index = mediaModel->index(row, 0, QModelIndex());
        if (index.isValid()) { // add only one itemData
            if (index.data(MediaModel::DotDotRole).toBool())
                return;
            beginInsertRows(QModelIndex(), m_data.count(), m_data.count());
            m_data.append(mediaModel->itemData(index));
            endInsertRows();
        } else {
            beginInsertRows(QModelIndex(), m_data.count(), m_data.count() + mediaModel->rowCount() - 1);
            for (int i = 0; i < mediaModel->rowCount(); i++) {
                QModelIndex idx = mediaModel->index(i, 0, QModelIndex());
                if (idx.data(MediaModel::DotDotRole).toBool())
                    continue;

                m_data.append(mediaModel->itemData(idx));
            }
            endInsertRows();
        }
    } else {
        qWarning() << __PRETTY_FUNCTION__ << "non leaf level add is not handled yet";
    }

    if (m_currentIndex == -1 && !m_data.isEmpty()) {
        m_currentIndex = 0;
        emit currentIndexChanged();
    }

    DEBUG << "Playlist now has " << rowCount() << " items";
}

void Playlist::setCurrentIndex(int index)
{
    if (m_currentIndex == index)
        return;

    if (index >= rowCount() || index < 0) {
        DEBUG << "Invalid index " << index;
        return;
    }

    DEBUG << "Index changed to " << index;
    m_currentIndex = index;
    emit currentIndexChanged();
}

int Playlist::currentIndex() const
{
    return m_currentIndex;
}

int Playlist::next()
{
    int index;
    if (m_playMode == Shuffle) {
        index = int((qreal(qrand())/RAND_MAX)*rowCount());
    } else {
        index = m_currentIndex >= rowCount()-1 ? 0 : m_currentIndex+1;
    }

    setCurrentIndex(index);
    return m_currentIndex;
}

int Playlist::previous()
{
    int index = m_currentIndex <= 0 ? rowCount()-1 : m_currentIndex-1;
    setCurrentIndex(index);
    return m_currentIndex;
}

Playlist::PlayMode Playlist::playMode() const
{
    return m_playMode;
}

void Playlist::setPlayMode(Playlist::PlayMode mode)
{
    if (m_playMode != mode) {
        m_playMode = mode;
        emit playModeChanged();
    }
}

QString Playlist::mediaType() const
{
    return m_mediaType;
}

void Playlist::setMediaType(const QString &type)
{
    if (type == m_mediaType)
        return;

    beginResetModel();
    m_data.clear();
    endResetModel();

    QHash<int, QByteArray> roleToName;
    m_nameToRole.clear();
    MediaModel::getRoleNameMapping(type, &roleToName, &m_nameToRole);
    setRoleNames(roleToName);

    m_mediaType = type;
    emit mediaTypeChanged();
}

