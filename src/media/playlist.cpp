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

#include "mediamodel.h"
#include "backend.h"

#define DEBUG if (0) qDebug() << __PRETTY_FUNCTION__

Playlist::Playlist(QObject *parent)
    : QAbstractListModel(parent)
    , m_playMode(Normal)
    , m_driver(0)
    , m_currentIndex(-1)
{
}

QVariant Playlist::data(int index, const QString &role) const
{
    return data(createIndex(index, 0), getRoleByName(role));
}

QVariant Playlist::data(int index, int role) const
{
    return data(createIndex(index, 0), role);
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

int Playlist::add(const QModelIndex &index, PlaylistRoles role, DepthRoles depth)
{
    DEBUG << "add item" << index;

    if (!index.isValid()) {
        DEBUG << "index is not valid, epic fail";
        return -1; // we don't have a model() to work with
    }

    const MediaModel *model = qobject_cast<const MediaModel*>(index.model());
    if (!model) {
        DEBUG << "model of index is not a MediaModel";
        return -1;
    }

    // if new item is not same type make this playlist of new type
    if (model->mediaType() != m_mediaType)
        setMediaType(model->mediaType());

    if (role == Playlist::Replace) {
        beginResetModel();
        m_data.clear();
        endResetModel();
    }

    int start = rowCount()-1 < 0 ? 0 : rowCount()-1;
    int end = rowCount() < 0 ? 1 : rowCount();

    emit beginInsertRows(QModelIndex(), start, end);

    // Add the fields of the table as role names
    if (!m_driver)
        return -1;

    QSqlRecord record = m_driver->record(m_mediaType);
    if (record.isEmpty()) {
        DEBUG << "Table " << m_mediaType << " is not valid it seems";
        return -1;
    }

    int pos = -1;

    if (depth == Playlist::Single) {
        QHash<int, QVariant> dataset;
        dataset.insert(PreviewUrlRole, index.data(MediaModel::PreviewUrlRole));
        dataset.insert(Qt::DisplayRole, index.data(Qt::DisplayRole));

        for (int i = 0; i < record.count(); i++) {
            dataset.insert(FieldRolesBegin + i, index.data(MediaModel::FieldRolesBegin + i));
        }

        m_data.append(dataset);
        pos = m_data.count()-1;
    } else if (depth == Playlist::Flat || depth == Playlist::Recursive) {
        for(int i = 0; i < model->rowCount(); i++) {
            QModelIndex idx = model->index(i, 0, QModelIndex());
            if (idx.isValid()) {
                QHash<int, QVariant> dataset;
                dataset.insert(PreviewUrlRole, idx.data(MediaModel::PreviewUrlRole));
                dataset.insert(Qt::DisplayRole, idx.data(Qt::DisplayRole));

                for (int i = 0; i < record.count(); i++) {
                    dataset.insert(FieldRolesBegin + i, idx.data(MediaModel::FieldRolesBegin + i));
                }

                m_data.append(dataset);
                if (idx == index)
                    pos = m_data.count()-1;
            }
        }
    } else {
        qWarning() << __PRETTY_FUNCTION__ << "depth not handled, yet";
    }

    emit endInsertRows();

    DEBUG << "Playlist now has " << rowCount() << " items";

    return pos;
}

QModelIndex Playlist::index(int row) const
{
    return createIndex(row, 0);
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

Playlist::PlayModeRoles Playlist::playMode() const
{
    return m_playMode;
}

void Playlist::setPlayMode(Playlist::PlayModeRoles mode)
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

    m_mediaType = type;
    emit mediaTypeChanged();

    // Add the fields of the table as role names
    if (!m_driver)
        m_driver = Backend::instance()->mediaDatabase().driver();

    QSqlRecord record = m_driver->record(m_mediaType);
    if (record.isEmpty())
        qWarning() << "Table " << type << " is not valid it seems";

    QHash<int, QByteArray> hash = roleNames();
    hash.insert(PreviewUrlRole, "previewUrl");

    for (int i = 0; i < record.count(); i++) {
        hash.insert(FieldRolesBegin + i, record.fieldName(i).toUtf8());
    }

    setRoleNames(hash);
}

int Playlist::getRoleByName(const QString &roleName) const
{
    QHash<int, QByteArray> hash = roleNames();
    QHashIterator<int, QByteArray> i(hash);
     while (i.hasNext()) {
         i.next();
         if (i.value() == roleName) {
             return i.key();
         }
     }

    return -1;
}

