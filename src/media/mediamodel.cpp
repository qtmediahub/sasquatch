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

#include "mediamodel.h"
#include "mediascanner.h"
#include "dbreader.h"
#include "qmh-config.h"

#define DEBUG if (0) qDebug() << this << __PRETTY_FUNCTION__

MediaModel::MediaModel(QObject *parent)
    : QAbstractItemModel(parent), m_loading(false), m_loaded(false), m_reader(0), m_readerThread(0)
{
    m_refreshTimer.setInterval(Config::value("mediamodel-refresh-interval", 10000));
    connect(&m_refreshTimer, SIGNAL(timeout()), this, SLOT(refresh()));

    MediaScanner *scanner = MediaScanner::instance();
    connect(scanner, SIGNAL(scanStarted(QString)), this, SLOT(handleScanStarted(QString)));
    connect(scanner, SIGNAL(scanFinished(QString)), this, SLOT(handleScanFinished(QString)));
    connect(scanner, SIGNAL(searchPathRemoved(QString, QString)), this, SLOT(handleScanFinished(QString)));

    m_readerThread = new QThread(this);
    m_readerThread->start();
}

MediaModel::~MediaModel()
{
    if (m_reader) {
        m_reader->stop();
        m_reader->deleteLater();
        m_readerThread->quit();
        m_readerThread->wait();
    }
}

QString MediaModel::part() const
{
    return m_structure.split("|").value(m_cursor.length());
}

QString MediaModel::mediaType() const
{
    return m_mediaType;
}

void MediaModel::setMediaType(const QString &type)
{
    if (type == m_mediaType)
        return;

    DEBUG << type;

    m_mediaType = type;
    emit mediaTypeChanged();

    // Add the fields of the table as role names
    QSqlDriver *driver = QSqlDatabase::database(DEFAULT_DATABASE_CONNECTION_NAME).driver();
    QSqlRecord record = driver->record(m_mediaType);
    if (record.isEmpty())
        qWarning() << "Table " << type << " is not valid it seems";

    QHash<int, QByteArray> hash = roleNames();
    hash.insert(DotDotRole, "dotdot");
    hash.insert(IsLeafRole, "isLeaf");
    hash.insert(ModelIndexRole,"modelIndex");
    hash.insert(PreviewUrlRole, "previewUrl");

    for (int i = 0; i < record.count(); i++) {
        hash.insert(FieldRolesBegin + i, record.fieldName(i).toUtf8());
        m_fieldToRole.insert(record.fieldName(i), FieldRolesBegin + i);
    }

    setRoleNames(hash);

    reload();
}

QString MediaModel::structure() const
{
    return m_structure;
}

void MediaModel::setStructure(const QString &str)
{
    if (str == m_structure)
        return;
    DEBUG << str;
    m_structure = str;
    m_layoutInfo.clear();
    foreach(const QString &part, m_structure.split("|"))
        m_layoutInfo.append(part.split(","));

    reload();

    emit structureChanged();
}

void MediaModel::enter(int index)
{
    if (m_cursor.count() + 1 == m_layoutInfo.count() && !m_data.at(index)[DotDotRole].toBool() /* up on leaf node is OK */) {
        DEBUG << "Refusing to enter leaf node";
        return;
    }

    if (m_data.at(index)[DotDotRole].toBool() && !m_cursor.isEmpty()) {
        back();
        return;
    }

    DEBUG << "Entering " << index;
    m_cursor.append(m_data[index]);
    reload();
    emit partChanged();
}

void MediaModel::back(int count)
{
    if (count == 0 || m_cursor.count() < 1)
        return;

    if (m_cursor.count() > count) {
        for (int i = 0; i < count; ++i)
            m_cursor.removeLast();
    } else {
        m_cursor.clear();
    }
    reload();
    emit partChanged();
}

QVariant MediaModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role == ModelIndexRole)
        return qVariantFromValue(index);
    
    return m_data.value(index.row()).value(role);
}

QModelIndex MediaModel::index(int row, int col, const QModelIndex &parent) const
{
    if (parent.isValid())
        return QModelIndex();

    return createIndex(row, col);
}

QModelIndex MediaModel::parent(const QModelIndex &idx) const
{
    Q_UNUSED(idx);
    return QModelIndex();
}

int MediaModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_data.count();
}

int MediaModel::columnCount(const QModelIndex &idx) const
{
    Q_UNUSED(idx);
    return 1;
}

bool MediaModel::hasChildren(const QModelIndex &parent) const
{
    return !parent.isValid();
}

bool MediaModel::canFetchMore(const QModelIndex &parent) const
{
    if (parent.isValid() || m_mediaType.isEmpty() || m_layoutInfo.isEmpty()) {
        DEBUG << "false " << parent.isValid() << m_mediaType.isEmpty() << m_layoutInfo.isEmpty();
        return false;
    }
    DEBUG << (!m_loading && !m_loaded);
    return !m_loading && !m_loaded;
}

void MediaModel::fetchMore(const QModelIndex &parent)
{
    if (!canFetchMore(parent))
        return;

    DEBUG << "";

    m_loading = true;

    QSqlQuery q = buildQuery();
    DEBUG << q.lastQuery();
    QMetaObject::invokeMethod(m_reader, "execute", Qt::QueuedConnection, Q_ARG(QSqlQuery, q));
}

void MediaModel::createNewDbReader()
{    
    DEBUG << "";

    if (m_reader) {
        disconnect(m_reader, 0, this, 0);
        m_reader->stop();
        m_reader->deleteLater();
    }
    m_reader = new DbReader;
    m_reader->moveToThread(m_readerThread);

    QMetaObject::invokeMethod(m_reader, "initialize", Q_ARG(QSqlDatabase, QSqlDatabase::database(DEFAULT_DATABASE_CONNECTION_NAME)));
    connect(m_reader, SIGNAL(dataReady(DbReader *, QList<QSqlRecord>, void *)),
            this, SLOT(handleDataReady(DbReader *, QList<QSqlRecord>, void *)));
}

void MediaModel::reload()
{
    createNewDbReader();

    beginResetModel();
    m_loading = m_loaded = false;
    m_data.clear();
    endResetModel();
}

QHash<int, QVariant> MediaModel::dataFromRecord(const QSqlRecord &record) const
{
    QHash<int, QVariant> data;
    for (int j = 0; j < record.count(); j++) {
        int role = m_fieldToRole.value(record.fieldName(j));
        if (record.fieldName(j) == "uri")
            data.insert(role, QUrl::fromEncoded(record.value(j).toByteArray()));
        else
            data.insert(role, record.value(j));
    }

    // Provide 'display' role as , separated values
    QStringList cols = m_layoutInfo.value(m_cursor.count());
    QStringList displayString;
    for (int j = 0; j < cols.count(); j++) {
        displayString << record.value(cols[j]).toString();
    }
    data.insert(Qt::DisplayRole, displayString.join(", "));
    data.insert(DotDotRole, false);
    data.insert(IsLeafRole, m_cursor.count() + 1 == m_layoutInfo.count());

    data.insert(PreviewUrlRole, QUrl::fromEncoded(record.value("thumbnail").toByteArray()));
    return data;
}

void MediaModel::handleDataReady(DbReader *reader, const QList<QSqlRecord> &records, void *node)
{
    Q_ASSERT(reader == m_reader);
    Q_UNUSED(reader);
    Q_UNUSED(node);

    DEBUG << "Received response from db of size " << records.size();

    if (m_data.isEmpty()) {
        insertAll(records);
    } else {
        update(records);
    }

    m_loading = false;
    m_loaded = true;
}

void MediaModel::insertAll(const QList<QSqlRecord> &records)
{
    const bool addDotDot = !m_cursor.isEmpty();

    if (records.isEmpty() && !addDotDot)
        return;

    beginInsertRows(QModelIndex(), 0, records.count() + (addDotDot ? 0 : -1));

    for (int i = 0; i < records.count(); i++) {
        QHash<int, QVariant> data = dataFromRecord(records[i]);
        m_data.append(data);
    }

    if (addDotDot) {
        QHash<int, QVariant> data;
        data.insert(Qt::DisplayRole, tr(".."));
        data.insert(DotDotRole, true);
        data.insert(IsLeafRole, false);
        m_data.prepend(data);
    }

    endInsertRows();
}

int MediaModel::compareData(int idx, const QSqlRecord &record) const
{
    const QHash<int, QVariant> &curData = m_data[idx];
    QStringList cols = m_layoutInfo.value(m_cursor.count());
    foreach(const QString &col, cols) {
        const int role = m_fieldToRole.value(col);
        int cmp = QString::compare(curData.value(role).toString(), record.value(col).toString(), Qt::CaseInsensitive); // ## must use sqlite's compare
        if (cmp != 0)
            return cmp;
    }
    return 0;
}

void MediaModel::update(const QList<QSqlRecord> &records)
{
    const bool hasDotDot = !m_cursor.isEmpty();
    int old = (int)hasDotDot, shiny = 0;
    
    while (shiny < records.length()) {
        const QSqlRecord &record = records[shiny];
        int cmp = old == m_data.count() ? 1 : compareData(old, record);
        
        if (cmp == 0) {
            ++old;
            ++shiny;
        } else if (cmp < 0) {
            beginRemoveRows(QModelIndex(), old, old);
            m_data.removeAt(old);
            endRemoveRows();
        } else {
            beginInsertRows(QModelIndex(), old, old);
            m_data.insert(old, dataFromRecord(record));
            endInsertRows();
            ++old;
            ++shiny;
        }
    }

    if (old != m_data.count()) {
        beginRemoveRows(QModelIndex(), old, m_data.count()-1);
        m_data = m_data.mid(0, old);
        endRemoveRows();
    }
}

QSqlQuery MediaModel::buildQuery() const
{
    QSqlDatabase db = QSqlDatabase::database(DEFAULT_DATABASE_CONNECTION_NAME);
    QSqlDriver *driver = db.driver();

    QStringList escapedCurParts;
    QStringList curParts = m_layoutInfo[m_cursor.count()];
    for (int i = 0; i < curParts.count(); i++)
        escapedCurParts.append(driver->escapeIdentifier(curParts[i], QSqlDriver::FieldName));
    QString escapedCurPart = escapedCurParts.join(",");

    QSqlQuery query(db);
    query.setForwardOnly(true);

    QStringList placeHolders;
    const bool lastPart = m_cursor.count() == m_layoutInfo.count()-1;
    QString queryString;
    // SQLite allows us to select columns that are not present in the GROUP BY. We use this feature
    // to select thumbnails for non-leaf nodes
    queryString.append("SELECT *");
    queryString.append(" FROM " + driver->escapeIdentifier(m_mediaType, QSqlDriver::TableName));

    if (!m_cursor.isEmpty()) {
        QStringList where;
        for (int i = 0; i < m_cursor.count(); i++) {
            QStringList subParts = m_layoutInfo[i];
            for (int j = 0; j < subParts.count(); j++) {
                where.append(subParts[j] + " = ?");
                const int role = m_fieldToRole.value(subParts[j]);
                placeHolders << m_cursor[i].value(role).toString();
            }
        }

        queryString.append(" WHERE " + where.join(" AND "));
    }

    if (!lastPart)
        queryString.append(" GROUP BY " + escapedCurPart);

    queryString.append(" ORDER BY " + escapedCurPart + " COLLATE NOCASE");

    query.prepare(queryString);

    foreach(const QString &placeHolder, placeHolders)
        query.addBindValue(placeHolder);

    return query;
}

void MediaModel::handleScanStarted(const QString &type)
{
    if (type != m_mediaType)
        return;

    m_refreshTimer.start();
}

void MediaModel::handleScanFinished(const QString &type)
{
    if (type != m_mediaType)
        return;
    m_refreshTimer.stop();
    refresh();
}

void MediaModel::refresh()
{
    createNewDbReader();

    m_loading = true;
    m_loaded = false;

    QSqlQuery q = buildQuery();
    DEBUG << m_mediaType << q.lastQuery();
    QMetaObject::invokeMethod(m_reader, "execute", Qt::QueuedConnection, Q_ARG(QSqlQuery, q));
}

