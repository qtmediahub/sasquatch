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
#include "scopedtransaction.h"
#include "global.h"

#define DEBUG if (0) qDebug() << __PRETTY_FUNCTION__

Playlist::Playlist(QObject *parent)
    : QAbstractListModel(parent)
    , m_playMode(Normal)
    , m_currentIndex(-1)
    , m_wrapAround(false)
{
    m_saveTimer.setInterval(2000);
    m_saveTimer.setSingleShot(true);
    connect(&m_saveTimer, SIGNAL(timeout()), this, SLOT(saveToDatabase()));

    setRoleNames(MediaModel::roleToNameMapping());
}

QVariant Playlist::data(int index, const QString &role) const
{
    const QHash<QString, int> nameToRole = MediaModel::nameToRoleMapping();
    return m_data.value(index).value(nameToRole.value(role));
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

    saveLater();
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

    if (mediaModel->rowCount() == 0) {
        DEBUG << "empty model";
        return;
    }

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
        QSqlQuery query = mediaModel->leafNodesQuery(row);
        query.exec();
        QList<QMap<int, QVariant> > newData;
        while (query.next()) {
            QMap<int, QVariant> data = MediaModel::dynamicRolesDataFromRecord(query.record());
            data.insert(Qt::DisplayRole, query.record().value("title"));
            newData.append(data);
        }
        beginInsertRows(QModelIndex(), m_data.count(), m_data.count() + newData.count() - 1);
        m_data.append(newData);
        endInsertRows();
    }

    if (m_currentIndex == -1 && !m_data.isEmpty()) {
        m_currentIndex = 0;
        emit currentIndexChanged();
    }

    saveLater();
    DEBUG << "Playlist now has " << rowCount() << " items";
}

void Playlist::setCurrentIndex(int index)
{
    if (m_currentIndex == index)
        return;

    if (index >= rowCount()) { // index can be < 0 to "unset" the current index
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
    } else if (m_wrapAround) {
        index = m_currentIndex >= rowCount()-1 ? 0 : m_currentIndex+1;
    } else {
        index = m_currentIndex >= rowCount()-1 ? -1 : m_currentIndex+1;
    }

    setCurrentIndex(index);
    return m_currentIndex;
}

int Playlist::previous()
{
    int index;
    if (m_wrapAround) {
        index = m_currentIndex <= 0 ? rowCount()-1 : m_currentIndex-1;
    } else {
        index = m_currentIndex <= 0 ? -1 : m_currentIndex-1;
    }
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

void Playlist::loadFromDatabase()
{
    QSqlQuery query(QSqlDatabase::database(DEFAULT_DATABASE_CONNECTION_NAME));

    QStringList mediaTypes;

    query.prepare("SELECT DISTINCT media_type FROM playlist WHERE name=:playlist_name");
    query.bindValue(":playlist_name", m_name);
    query.next();
    while (query.next()) {
        mediaTypes << query.value(0).toString();
    }

    // FIXME: Ordering of playlist is lost
    QList<QMap<int, QVariant> > newData;
    foreach(const QString &mediaType, mediaTypes) {
        query.prepare(QString("SELECT %1.* FROM %1 JOIN playlist ON playlist.media_id = %1.id WHERE playlist.name=:playlist_name").arg(mediaType));
        query.bindValue(":playlist_name", m_name);

        query.exec();

        while (query.next()) {
            QMap<int, QVariant> data = MediaModel::dynamicRolesDataFromRecord(query.record());
            data.insert(Qt::DisplayRole, query.record().value("title"));
            newData.append(data);
        }
    }

    beginInsertRows(QModelIndex(), 0, newData.count()-1);
    m_data = newData;
    endInsertRows();
}

void Playlist::saveLater()
{
    if (!m_name.isEmpty())
        m_saveTimer.start();
}

void Playlist::saveToDatabase()
{
    QSqlDatabase db = QSqlDatabase::database(DEFAULT_DATABASE_CONNECTION_NAME);
    ScopedTransaction transaction(db);
    QSqlQuery query(db);
    query.prepare("DELETE FROM playlist WHERE name = ?");
    query.addBindValue(m_name);
    if (!query.exec()) {
        qWarning() << "Failed to remove playlist";  
        return;
    }
    query.prepare("INSERT INTO playlist (name, media_type, media_id) VALUES (:name, :media_type, :media_id)");
    for (int i = 0; i < m_data.count(); i++) {
        query.bindValue(":name", m_name);
        query.bindValue(":media_type", data(i, "mediaType"));
        query.bindValue(":media_id", data(i, "id"));
        if (!query.exec()) {
            qWarning() << "Failed to insert into playlist"; 
        }
    }
}

void Playlist::setName(const QString &name)
{
    if (m_name == name)
        return;

    if (!m_name.isEmpty() && m_saveTimer.isActive()) // pending save
        saveToDatabase();

    m_name = name;
    emit nameChanged();
    beginResetModel();
    m_data.clear();
    endResetModel();

    loadFromDatabase();
}

QString Playlist::name() const
{
    return m_name;
}

void Playlist::setWrapAround(bool wrap)
{
    if (m_wrapAround == wrap)
        return;

    m_wrapAround = wrap;
    emit wrapAroundChanged();
}

bool Playlist::wrapAround() const
{
    return m_wrapAround;
}

