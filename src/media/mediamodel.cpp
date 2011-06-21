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

QString MediaModel::mediaType() const
{
    return m_mediaType;
}

void MediaModel::setMediaType(const QString &type)
{
    m_mediaType = type;
    emit mediaTypeChanged();

    beginResetModel();
    initialize();
    endResetModel();

    QSqlDriver *driver = Backend::instance()->mediaDatabase().driver();
    QSqlRecord record = driver->record(m_mediaType);
    if (record.isEmpty())
        qWarning() << "Table " << type << " is not valid it seems";

    QHash<int, QByteArray> hash = roleNames();

    for (int i = 0; i < record.count(); i++) {
        hash.insert(Qt::UserRole + i, record.fieldName(i).toUtf8());
    }

    setRoleNames(hash);
}

void MediaModel::addSearchPath(const QString &path, const QString &name)
{
    QMetaObject::invokeMethod(MediaScanner::instance(), "addSearchPath", Qt::QueuedConnection, Q_ARG(QString, "picture"), Q_ARG(QString, path), Q_ARG(QString, name));
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
    emit structureChanged();

    beginResetModel();
    initialize();
    endResetModel();
}

void MediaModel::enter(int index)
{
    Q_UNUSED(index);

    if (m_cursor.count() + 1 == m_structure.split("|").count() &&  index != 0 /* up on leaf node is OK */) {
        DEBUG << "Refusing to enter leaf node";
        return;
    }

    if (index == 0 && !m_cursor.isEmpty()) {
        back();
        return;
    }

    DEBUG << "Entering " << index;

    m_cursor.append(m_data[index]);

    beginResetModel();
    initialize();
    endResetModel();

    // fetchMore(QModelIndex());
}

void MediaModel::back()
{
    beginResetModel();
    m_cursor.removeLast();
    initialize();
    endResetModel();
}

QVariant MediaModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();
    
    const QHash<QString, QVariant> &data = m_data[index.row()];
    const QHash<int, QByteArray> hash = roleNames();
    return data.value(hash.value(role));
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
    if (parent.isValid() || m_mediaType.isEmpty() || m_structure.isEmpty())
        return false;
    return !m_loading && !m_loaded;
}

void MediaModel::fetchMore(const QModelIndex &parent)
{
    if (!canFetchMore(parent))
        return;

    initialize();

    QSqlQuery q = query();
    DEBUG << q.lastQuery();

    QMetaObject::invokeMethod(m_reader, "execute", Qt::QueuedConnection, Q_ARG(QSqlQuery, q));
}

void MediaModel::initialize()
{
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
}

void MediaModel::handleDataReady(DbReader *reader, const QList<QSqlRecord> &records, void *node)
{
    Q_ASSERT(reader == m_reader);
    Q_UNUSED(reader);
    Q_UNUSED(node);

    DEBUG << "Received response from db of size " << records.size();

    if (!m_cursor.isEmpty()) {
        beginInsertRows(QModelIndex(), 0, records.count());
        QHash<QString, QVariant> data;
        data.insert("display", tr(".."));
        m_data.append(data);
    } else {
        beginInsertRows(QModelIndex(), 0, records.count() - 1);
    }

    for (int i = 0; i < records.count(); i++) {
        QHash<QString, QVariant> data;
        for (int j = 0; j < records[i].count(); j++) {
            data.insert(records[i].fieldName(j), records[i].value(j));
        }

        QStringList cols = m_structure.split("|").value(m_cursor.count()).split(",");
        QStringList displayString;
        for (int j = 0; j < cols.count(); j++) {
            displayString << records[i].value(cols[j]).toString();
        }
        data.insert("display", displayString.join(", "));

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

// ## BIG FAT FIXME: Fix the string escaping to prevent sql injection!
QSqlQuery MediaModel::query()
{
    QString q;
    QStringList parts = m_structure.split("|");
    QString curPart = parts[m_cursor.count()];

    QSqlQuery query(Backend::instance()->mediaDatabase());
    query.setForwardOnly(true);

    if (parts.count() == 1) {
        query.prepare(QString("SELECT * FROM %1").arg(m_mediaType));
        return query;
    }

    QStringList where;
    for (int i = 0; i < m_cursor.count(); i++) {
        QString part = parts[i];
        QStringList subParts = part.split(",");
        for (int j = 0; j < subParts.count(); j++)
            where.append(subParts[j] + " = '" + m_cursor[i].value(subParts[j]).toString() + "'");
    }
    QString conditions = where.join(" AND ");

    if (conditions.isEmpty()) {
        query.prepare(QString("SELECT * FROM %1 GROUP BY %2").arg(m_mediaType).arg(curPart));
    } else {
        query.prepare(QString("SELECT DISTINCT %1 FROM %2 WHERE %3").arg(curPart).arg(m_mediaType).arg(conditions));
    }

    return query;
}

