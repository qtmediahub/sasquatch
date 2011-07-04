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
{
}

Playlist::~Playlist()
{
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

QVariant Playlist::add(const QModelIndex &index, PlaylistRoles role, DepthRoles depth)
{
    DEBUG << "add item" << index;

    if (!index.isValid()) {
        DEBUG << "index is not valid, epic fail";
        return QVariant(); // we don't have a model() to work with
    }

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
        return QVariant();

    QSqlRecord record = m_driver->record(m_mediaType);
    if (record.isEmpty()) {
        DEBUG << "Table " << m_mediaType << " is not valid it seems";
        return QVariant();
    }

    QHash<int, QVariant> dataset;
    dataset.insert(PreviewUrlRole, index.data(MediaModel::PreviewUrlRole));

    for (int i = 0; i < record.count(); i++) {
        dataset.insert(FieldRolesBegin + i, index.data(MediaModel::FieldRolesBegin + i));
    }

    m_data.append(dataset);

    emit endInsertRows();

    DEBUG << "Playlist now has " << rowCount() << " items";

    return qVariantFromValue(createIndex(m_data.count()-1, 0));
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

