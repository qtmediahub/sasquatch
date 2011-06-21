#include "mediamodel.h"
#include "mediascanner.h"
#include "dbreader.h"
#include "backend.h"

#define DEBUG if (1) qDebug() << __PRETTY_FUNCTION__

MediaModel::MediaModel(QObject *parent)
    : QAbstractItemModel(parent), m_depth(0), m_loading(false), m_loaded(false), m_reader(0), m_readerThread(0)
{
//    PictureParser *parser = new PictureParser;
//    MediaScanner::instance()->addParser(parser);
//    connect(parser, SIGNAL(databaseUpdated(QList<QSqlRecord>)),
//            this, SLOT(handleDatabaseUpdated(QList<QSqlRecord>)));

    m_mediaType = "picture";
}

MediaModel::~MediaModel()
{
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
    m_depth = 0; // reset depth
    emit structureChanged();
}

int MediaModel::depth() const
{
    return m_depth;
}

void MediaModel::enter(int index)
{
    Q_UNUSED(index);
}

void MediaModel::back()
{
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
    if (parent.isValid())
        return false;
    return !m_loading && !m_loaded;
}

void MediaModel::fetchMore(const QModelIndex &parent)
{
    if (!canFetchMore(parent))
        return;

    initialize();

    QMetaObject::invokeMethod(m_reader, "execute", Qt::QueuedConnection, Q_ARG(QSqlQuery, query()));
}

void MediaModel::initialize()
{
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
}

void MediaModel::handleDataReady(DbReader *reader, const QList<QSqlRecord> &records, void *node)
{
    Q_ASSERT(reader == m_reader);
    Q_UNUSED(reader);
    Q_UNUSED(node);

    DEBUG << "Received response from db of size " << records.size();

    beginInsertRows(QModelIndex(), 0, records.count()-1);

    for (int i = 0; i < records.count(); i++) {
        QHash<QString, QVariant> data;
        for (int j = 0; j < records[i].count(); j++) {
            data.insert(records[i].fieldName(j), records[i].value(j));
        }
        
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

QString MediaModel::query()
{
    QString q;
    QStringList parts = m_structure.split("|");
    if (m_depth == parts.count() - 1) { // last part
        return QString("SELECT * FROM %1").arg(m_mediaType);
    }
    return QString();
}

