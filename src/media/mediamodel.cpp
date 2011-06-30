#include "mediamodel.h"
#include "mediascanner.h"
#include "dbreader.h"
#include "backend.h"

#define DEBUG if (1) qDebug() << __PRETTY_FUNCTION__

MediaModel::MediaModel(QObject *parent)
    : QAbstractItemModel(parent), m_loading(false), m_loaded(false), m_reader(0), m_readerThread(0)
{
}

MediaModel::~MediaModel()
{
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

    initialize();

    m_mediaType = type;
    emit mediaTypeChanged();

    // Add the fields of the table as role names
    QSqlDriver *driver = Backend::instance()->mediaDatabase().driver();
    QSqlRecord record = driver->record(m_mediaType);
    if (record.isEmpty())
        qWarning() << "Table " << type << " is not valid it seems";

    QHash<int, QByteArray> hash = roleNames();
    hash.insert(DotDotRole, "dotdot");
    hash.insert(IsLeafRole, "isLeaf");

    for (int i = 0; i < record.count(); i++) {
        hash.insert(FieldRolesBegin + i, record.fieldName(i).toUtf8());
    }

    setRoleNames(hash);
}

void MediaModel::addSearchPath(const QString &path, const QString &name)
{
    if (m_mediaType.isEmpty()) {
        qWarning() << m_mediaType << " is not set yet";
        return;
    }

    QMetaObject::invokeMethod(MediaScanner::instance(), "addSearchPath", Qt::QueuedConnection, 
                              Q_ARG(QString, m_mediaType), Q_ARG(QString, path), Q_ARG(QString, name));
}

void MediaModel::removeSearchPath(int index)
{
    Q_UNUSED(index);
}

QString MediaModel::structure() const
{
    return m_structure;
}

void MediaModel::setStructure(const QString &str)
{
    m_structure = str;
    initialize();
    m_layoutInfo.clear();
    foreach(const QString &part, m_structure.split("|"))
        m_layoutInfo.append(part.split(","));

    emit structureChanged();
}

void MediaModel::enter(int index)
{
    if (m_cursor.count() + 1 == m_layoutInfo.count() && index != 0 /* up on leaf node is OK */) {
        DEBUG << "Refusing to enter leaf node";
        return;
    }

    if (index == 0 && !m_cursor.isEmpty()) {
        back();
        return;
    }

    DEBUG << "Entering " << index;
    m_cursor.append(m_data[index]);
    initialize();
    emit partChanged();
}

void MediaModel::back()
{
    m_cursor.removeLast();
    initialize();
    emit partChanged();
}

QVariant MediaModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();
    
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
    if (parent.isValid() || m_mediaType.isEmpty() || m_layoutInfo.isEmpty())
        return false;
    return !m_loading && !m_loaded;
}

void MediaModel::fetchMore(const QModelIndex &parent)
{
    if (!canFetchMore(parent))
        return;

    m_loading = true;

    QSqlQuery q = query();
    DEBUG << q.lastQuery();
    QMetaObject::invokeMethod(m_reader, "execute", Qt::QueuedConnection, Q_ARG(QSqlQuery, q));
}

void MediaModel::initialize()
{
    DEBUG << "";

    beginResetModel();

    m_data.clear();

    DbReader *newReader = new DbReader;
    if (m_reader) {
        disconnect(m_reader, 0, this, 0);
        m_reader->stop();
        m_reader->deleteLater();
    }
    m_reader = newReader;

    if (!m_readerThread) {
        m_readerThread = new QThread(this);
        m_readerThread->start();
    }
    m_reader->moveToThread(m_readerThread);
    QMetaObject::invokeMethod(m_reader, "initialize", Q_ARG(QSqlDatabase, Backend::instance()->mediaDatabase()));
    connect(m_reader, SIGNAL(dataReady(DbReader *, QList<QSqlRecord>, void *)),
            this, SLOT(handleDataReady(DbReader *, QList<QSqlRecord>, void *)));

    m_loading = m_loaded = false;

    endResetModel();
}

void MediaModel::handleDataReady(DbReader *reader, const QList<QSqlRecord> &records, void *node)
{
    Q_ASSERT(reader == m_reader);
    Q_UNUSED(reader);
    Q_UNUSED(node);

    DEBUG << "Received response from db of size " << records.size();

    if (records.isEmpty())
        return;

    if (!m_cursor.isEmpty()) {
        beginInsertRows(QModelIndex(), 0, records.count());
        QHash<int, QVariant> data;
        data.insert(Qt::DisplayRole, tr(".."));
        data.insert(DotDotRole, true);
        data.insert(IsLeafRole, false);
        m_data.append(data);
    } else {
        beginInsertRows(QModelIndex(), 0, records.count() - 1);
    }

    const bool isLeaf = m_cursor.count() + 1 == m_layoutInfo.count();

    QSqlDriver *driver = Backend::instance()->mediaDatabase().driver();
    const QSqlRecord tableRecord = driver->record(m_mediaType);

    for (int i = 0; i < records.count(); i++) {
        QHash<int, QVariant> data;
        for (int j = 0; j < records[i].count(); j++) {
            int idx = tableRecord.indexOf(records[i].fieldName(j));
            data.insert(FieldRolesBegin + idx, records[i].value(j));
        }

        // Provide 'display' role as , separated values
        QStringList cols = m_layoutInfo.value(m_cursor.count());
        QStringList displayString;
        for (int j = 0; j < cols.count(); j++) {
            displayString << records[i].value(cols[j]).toString();
        }
        data.insert(Qt::DisplayRole, displayString.join(", "));
        data.insert(DotDotRole, false);
        data.insert(IsLeafRole, isLeaf);

        m_data.append(data);
    }

    m_loading = false;
    m_loaded = true;

    endInsertRows();
}

void MediaModel::handleDatabaseUpdated(const QList<QSqlRecord> &records)
{
    Q_UNUSED(records);
    // not implemented yet
}

QSqlQuery MediaModel::query()
{
    QSqlDriver *driver = Backend::instance()->mediaDatabase().driver();

    QStringList escapedCurParts;
    QStringList curParts = m_layoutInfo[m_cursor.count()];
    for (int i = 0; i < curParts.count(); i++)
        escapedCurParts.append(driver->escapeIdentifier(curParts[i], QSqlDriver::FieldName));
    QString escapedCurPart = escapedCurParts.join(",");

    QSqlQuery query(Backend::instance()->mediaDatabase());
    query.setForwardOnly(true);

    QStringList placeHolders;
    const bool lastPart = m_cursor.count() == m_layoutInfo.count()-1;
    QString queryString;
    queryString.append("SELECT " + (lastPart ? QString("*") : escapedCurPart));
    queryString.append(" FROM " + driver->escapeIdentifier(m_mediaType, QSqlDriver::TableName));

    if (!m_cursor.isEmpty()) {
        QStringList where;
        const QSqlRecord tableRecord = driver->record(m_mediaType);
        for (int i = 0; i < m_cursor.count(); i++) {
            QStringList subParts = m_layoutInfo[i];
            for (int j = 0; j < subParts.count(); j++) {
                where.append(subParts[j] + " = ?");
                const int role = FieldRolesBegin + tableRecord.indexOf(subParts[j]);
                placeHolders << m_cursor[i].value(role).toString();
            }
        }

        queryString.append(" WHERE " + where.join(" AND "));
    }

    if (!lastPart)
        queryString.append(" GROUP BY " + escapedCurPart);

    queryString.append(" ORDER BY " + escapedCurPart);

    query.prepare(queryString);

    foreach(const QString &placeHolder, placeHolders)
        query.addBindValue(placeHolder);

    return query;
}

