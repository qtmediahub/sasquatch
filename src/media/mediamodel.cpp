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

#include "mediamodel.h"
#include "mediascanner.h"
#include "dbreader.h"
#include "skinruntime.h"

#define DEBUG if (0) qDebug() << this << __PRETTY_FUNCTION__
#define WARNING if (1) qWarning() << this << __PRETTY_FUNCTION__

MediaModel::MediaModel(QObject *parent):
    QAbstractItemModel(parent),
    m_loading(false),
    m_loaded(false),
    m_reader(0),
    m_readerThread(0),
    m_autoForward(false),
    m_dotDotPosition(MediaModel::Beginning)
{
    setRoleNames(MediaModel::roleToNameMapping());

    // TODO this is single point for GlobalSettings singleton usage
    // we know that we have a instance already
    m_settings = GlobalSettings::instance();

    m_refreshTimer.setInterval(m_settings->value(GlobalSettings::MediaRefreshInterval).toInt());
    connect(&m_refreshTimer, SIGNAL(timeout()), this, SLOT(refresh()));

    MediaScanner *scanner = MediaScanner::instance(m_settings);
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

int MediaModel::s_currentDynamicRole = MediaModel::FieldRolesBegin;
QHash<int, QByteArray> MediaModel::s_roleToName;
QHash<QString, int> MediaModel::s_nameToRole;

void MediaModel::createStaticRoleNameMapping()
{
    struct {
        int role; 
        const char *name;
    } roleNames[] = {
        { Qt::DisplayRole, "display" },
        { DotDotRole, "dotdot" },
        { MediaTypeRole, "mediaType" },
        { IsLeafRole, "isLeaf"},
        { ModelIndexRole, "modelIndex"},
        { PreviewUrlRole, "previewUrl"},
        { 0, NULL }
    };

    for (int i = 0; roleNames[i].name != NULL; ++i) {
        s_roleToName.insert(roleNames[i].role, roleNames[i].name);
        s_nameToRole.insert(roleNames[i].name, roleNames[i].role);
    }
}

void MediaModel::createDynamicRoleNameMapping(const QString &table)
{
    // Add the fields of the table as role names
    QSqlDriver *driver = QSqlDatabase::database(DEFAULT_DATABASE_CONNECTION_NAME).driver();
    QSqlRecord record = driver->record(table);
    if (record.isEmpty()) {
        qWarning() << "Table " << table << " is not valid it seems";
        return;
    }

    for (int i = 0; i < record.count(); i++) {
        QByteArray field = record.fieldName(i).toUtf8();
        if (s_nameToRole.contains(field))
            continue;
        s_nameToRole.insert(field, s_currentDynamicRole);
        s_roleToName.insert(s_currentDynamicRole, field);
        ++s_currentDynamicRole;
    }
}

QHash<int, QByteArray> MediaModel::roleToNameMapping()
{
    return s_roleToName;
}

QHash<QString, int> MediaModel::nameToRoleMapping()
{
    return s_nameToRole;
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

    reload();
}

QString MediaModel::sqlCondition() const
{
    return m_sqlCondition;
}

void MediaModel::setSqlCondition(const QString &sqlCondition)
{
    if (sqlCondition == m_sqlCondition)
        return;

    DEBUG << sqlCondition;

    m_sqlCondition = sqlCondition;
    emit sqlConditionChanged();

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
    m_cursor.clear();
    m_structure = str;
    m_layoutInfo.clear();
    foreach(const QString &part, m_structure.split("|"))
        m_layoutInfo.append(part.split(","));

    reload();

    emit structureChanged();
}

void MediaModel::setDotDotPosition(MediaModel::DotDotPosition position)
{
    if (m_dotDotPosition == position)
        return;

    const bool hasDotDot = !m_cursor.isEmpty() && m_dotDotPosition != MediaModel::Nowhere;
    if (hasDotDot) {
        emit layoutAboutToBeChanged();
        if (m_dotDotPosition == MediaModel::Beginning) {
            m_data.append(m_data.takeFirst());
        } else {
            m_data.prepend(m_data.takeLast());
        }
        emit layoutChanged();
    }
    m_dotDotPosition = position;
    emit dotDotPositionChanged();
}

MediaModel::DotDotPosition MediaModel::dotDotPosition() const
{
    return m_dotDotPosition;
}

void MediaModel::enter(int index)
{
    if (m_data.size() <= index || (m_cursor.count() + 1 == m_layoutInfo.count() && !m_data.at(index)[DotDotRole].toBool()) /* up on leaf node is OK */) {
        WARNING << "Refusing to enter leaf node";
        return;
    }

    if (m_data.at(index)[DotDotRole].toBool() && !m_cursor.isEmpty()) {
        m_autoForward = false;
        back();
        return;
    }

    m_autoForward = true;
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

QMap<int, QVariant> MediaModel::itemData(const QModelIndex &index) const
{
    return m_data.value(index.row());
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

    QPair<QString, QStringList> q = buildQuery(m_cursor, false /* forceLastPart */);
    DEBUG << q.first << q.second;
    QMetaObject::invokeMethod(m_reader, "execute", Qt::QueuedConnection, Q_ARG(QString, q.first), Q_ARG(QStringList, q.second));
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

QMap<int, QVariant> MediaModel::dynamicRolesDataFromRecord(const QSqlRecord &record)
{
    QMap<int, QVariant> data;
    for (int j = 0; j < record.count(); j++) {
        int role = s_nameToRole.value(record.fieldName(j));
        if (record.fieldName(j) == "uri")
            data.insert(role, QUrl::fromEncoded(record.value(j).toByteArray()));
        else
            data.insert(role, record.value(j));
    }

    data.insert(PreviewUrlRole, QUrl::fromEncoded(record.value("thumbnail").toByteArray()));
    return data;
}

QString MediaModel::displayStringFromRecord(const QSqlRecord &record) const
{
    // Provide 'display' role as , separated values
    QStringList cols = m_layoutInfo.value(m_cursor.count());
    QStringList displayString;
    for (int j = 0; j < cols.count(); j++) {
        displayString << record.value(cols[j]).toString();
    }
    return displayString.join(", ");
}

QMap<int, QVariant> MediaModel::dataFromRecord(const QSqlRecord &record) const
{
    QMap<int, QVariant> data = dynamicRolesDataFromRecord(record);
    data.insert(Qt::DisplayRole, displayStringFromRecord(record));
    data.insert(DotDotRole, false);
    data.insert(IsLeafRole, isLeafLevel());
    data.insert(MediaTypeRole, m_mediaType);
    return data;
}

void MediaModel::handleDataReady(DbReader *reader, const QList<QSqlRecord> &records, void *node)
{
    Q_UNUSED(reader);
    Q_UNUSED(node);

    // Someone please check why we need an assert here...this let us bail out often on startup
    // the simple return works as far as I've tested
    //    Q_ASSERT(reader == m_reader);
    if (reader != m_reader) {
        DEBUG << "Received data ready from wrong DbReader instance";
        return;
    }

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
    const bool addDotDot = !m_cursor.isEmpty() && m_dotDotPosition != MediaModel::Nowhere;

    if (records.isEmpty() && !addDotDot)
        return;

    beginInsertRows(QModelIndex(), 0, records.count() + (addDotDot ? 0 : -1));

    for (int i = 0; i < records.count(); i++) {
        m_data.append(dataFromRecord(records[i]));
    }

    if (addDotDot) {
        QMap<int, QVariant> data;
        data.insert(Qt::DisplayRole, tr(".."));
        data.insert(DotDotRole, true);
        data.insert(IsLeafRole, false);
        data.insert(MediaTypeRole, QString());
        if (m_dotDotPosition == MediaModel::Beginning) {
            m_data.prepend(data);
        } else {
            m_data.append(data);
        }
    }

    endInsertRows();

    if (records.count() == 1 && m_data[addDotDot && m_dotDotPosition == MediaModel::Beginning ? 1 : 0].value(Qt::DisplayRole).toString().isEmpty()) {
        // Automatically move forward or backward if the model has just one item and that item is null
        if (m_autoForward) {
            enter(addDotDot && m_dotDotPosition == MediaModel::Beginning ? 1 : 0);
        } else {
            Q_ASSERT(addDotDot);
            back();
        }
    }
}

int MediaModel::compareData(int idx, const QSqlRecord &record) const
{
    // TODO girish please confirm
    if (idx >= m_data.size()) {
        DEBUG << "index out of data list size";
        return 0;
    }

    const QMap<int, QVariant> &curData = m_data[idx];
    QStringList cols = m_layoutInfo.value(m_cursor.count());
    foreach(const QString &col, cols) {
        const int role = s_nameToRole.value(col);
        int cmp = QString::compare(curData.value(role).toString(), record.value(col).toString(), Qt::CaseInsensitive); // ## must use sqlite's compare
        if (cmp != 0)
            return cmp;
    }
    return 0;
}

void MediaModel::update(const QList<QSqlRecord> &records)
{
    const bool hasDotDot = !m_cursor.isEmpty() && m_dotDotPosition != MediaModel::Nowhere;
    const int oldStart = hasDotDot && m_dotDotPosition == MediaModel::Beginning ? 1 : 0;
    const int oldEnd = hasDotDot && m_dotDotPosition == MediaModel::End ? m_data.count() - 1 : m_data.count();

    int old = oldStart;
    int shiny = 0;
    
    while (shiny < records.length()) {
        const QSqlRecord &record = records[shiny];
        int cmp = old == oldEnd ? 1 : compareData(old, record);
        
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

    if (old != oldEnd) {
        beginRemoveRows(QModelIndex(), old, oldEnd-1);
        for (int i = old; i <= oldEnd-1; i++)
            m_data.removeAt(old);
        endRemoveRows();
    }
}

QPair<QString, QStringList> MediaModel::buildQuery(const QList<QMap<int, QVariant> > &cursor, bool forceLastPart) const
{
    QSqlDatabase db = QSqlDatabase::database(DEFAULT_DATABASE_CONNECTION_NAME);
    QSqlDriver *driver = db.driver();

    QString queryString;
    // SQLite allows us to select columns that are not present in the GROUP BY. We use this feature
    // to select thumbnails for non-leaf nodes
    queryString.append("SELECT *");
    queryString.append(" FROM " + driver->escapeIdentifier(m_mediaType, QSqlDriver::TableName));

    QStringList placeHolders;
    if (!cursor.isEmpty()) {
        QStringList where;
        for (int i = 0; i < cursor.count(); i++) {
            QStringList subParts = m_layoutInfo[i];
            for (int j = 0; j < subParts.count(); j++) {
                const int role = s_nameToRole.value(subParts[j]);
                QString value = cursor[i].value(role).toString();
                if (value.isEmpty()) {
                    where.append(subParts[j] + " is NULL");
                } else {
                    placeHolders << value;
                    where.append(subParts[j] + " = ?");
                }
            }
        }

        queryString.append(" WHERE " + where.join(" AND "));
    }

    // add WHERE for the model wide condition
    if (!m_sqlCondition.isEmpty()) {
        if (!queryString.contains("WHERE"))
            queryString.append(" WHERE ");
        else
            queryString.append(" AND ");
        queryString.append(m_sqlCondition);
    }

    QStringList escapedCurParts;
    QStringList curParts = m_layoutInfo[cursor.count()];
    for (int i = 0; i < curParts.count(); i++)
        escapedCurParts.append(driver->escapeIdentifier(curParts[i], QSqlDriver::FieldName));
    QString escapedCurPart = escapedCurParts.join(",");

    const bool lastPart = forceLastPart || cursor.count() == m_layoutInfo.count()-1;
    if (!lastPart)
        queryString.append(" GROUP BY " + escapedCurPart);

    queryString.append(" ORDER BY " + escapedCurPart + " COLLATE NOCASE");

    return qMakePair(queryString, placeHolders);
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

    QPair<QString, QStringList> q = buildQuery(m_cursor, false /* forceLastPart */);
    DEBUG << m_mediaType << q.first << q.second;
    QMetaObject::invokeMethod(m_reader, "execute", Qt::QueuedConnection, Q_ARG(QString, q.first), Q_ARG(QStringList, q.second));
}

bool MediaModel::isLeafLevel() const
{
    return m_cursor.count() + 1 == m_layoutInfo.count();
}

QSqlQuery MediaModel::leafNodesQuery(int row) const
{
    QList<QMap<int, QVariant> > cursor = m_cursor;
    cursor.append(m_data.value(row));
    QPair<QString, QStringList> q = buildQuery(cursor, true /* forceLastPart */);
    QSqlQuery query(QSqlDatabase::database(DEFAULT_DATABASE_CONNECTION_NAME));
    query.setForwardOnly(true);
    query.prepare(q.first);
    foreach(const QString &binding, q.second)
        query.addBindValue(binding);
    return query;
}

