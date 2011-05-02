#include "videomodel.h"
#include "videoparser.h"
#include "mediascanner.h"
#include "scopedtransaction.h"
#include "backend.h"
#include "dbreader.h"
#include <QtSql>

#define DEBUG if (1) qDebug() << __PRETTY_FUNCTION__

static const int LIMIT = 200;

static QSqlQuery readMoviesQuery(const QString &lastTitle, const QStringList &ignoreIds)
{
    DEBUG << "Reading movies from " << lastTitle;

    QSqlQuery query(Backend::instance()->mediaDatabase());
    query.setForwardOnly(true);
    if (!ignoreIds.isEmpty()) {
        // This query will result in duplicates because of >=. But it's necessary to ensure that we retreive all movies with the same
        // title and we were split up midway.
        // If you change the query here, change the scanner code also
        query.prepare(QString("SELECT id, filepath, title, thumbnail FROM video WHERE title >= :title AND id NOT IN (%1) ORDER BY title LIMIT :limit").arg(ignoreIds.join(",")));
        query.bindValue(":title", lastTitle);
    } else {
        query.prepare("SELECT id, filepath, title, thumbnail FROM video ORDER BY title LIMIT :limit");
    }
    query.bindValue(":limit", LIMIT);
    return query;
}

VideoModel::VideoModel(QObject *parent)
    : QAbstractItemModel(parent), m_root(0), m_reader(0), m_readerThread(0),
      m_groupBy(NoGrouping), m_readerResponsePending(0)
{
    QHash<int, QByteArray> hash = roleNames();
    hash[PreviewUrlRole] = "previewUrl";
    setRoleNames(hash);

    VideoParser *parser = new VideoParser;
    MediaScanner::instance()->addParser(parser);
    connect(parser, SIGNAL(databaseUpdated(QList<QSqlRecord>)),
            this, SLOT(handleDatabaseUpdated(QList<QSqlRecord>)));

    m_root = new Node(0, Node::RootNode);

    groupBy(NoGrouping);
}

VideoModel::~VideoModel()
{
    DEBUG << "Stopping threads";

    if (m_readerThread) {
        m_reader->stop();
        m_readerThread->quit();
        m_readerThread->wait();

        DEBUG << "Reader stopped";
    }

    delete m_reader;
    delete m_root;
}

QVariant VideoModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    Node *node = static_cast<Node *>(index.internalPointer());

    switch (role) {
    case Qt::DisplayRole: 
        return node->text + " (" + node->filePath + ')';
    case PreviewUrlRole: 
        if (!node->hasThumbnail)
            return QVariant();
        return QString("image://videomodel/movie/%1").arg(node->id);
    default: 
        return QVariant();
    }
}

QModelIndex VideoModel::index(int row, int col, const QModelIndex &parent) const
{
    if (col != 0 || row < 0)
        return QModelIndex();
    Node *parentNode = parent.isValid() ? static_cast<Node *>(parent.internalPointer()) : m_root;
    if (parentNode->type == Node::DotDot) {
        parentNode = parentNode->parent->parent;
        if (!parentNode)
            parentNode = m_root;
    }
    if (row >= parentNode->children.count())
        return QModelIndex();
    Q_ASSERT(parentNode->children[row] != 0);
    return createIndex(row, col, parentNode->children[row]);
}

QModelIndex VideoModel::indexForNode(VideoModel::Node *node) const
{
    return node == m_root
            ? QModelIndex()
            : createIndex(node->parent->children.indexOf(node), 0, node);
}

QModelIndex VideoModel::parent(const QModelIndex &idx) const
{
    if (!idx.isValid())
        return QModelIndex();
    Node *node = static_cast<Node *>(idx.internalPointer());
    Node *parent = node->parent;
    Node *grandParent = parent && parent->parent ? parent->parent : m_root;
    return createIndex(grandParent->children.indexOf(parent), idx.column(), parent);
}

int VideoModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return m_root->children.count();

    Node *node = static_cast<Node *>(parent.internalPointer());
    if (node->type == Node::DotDot) {
        Node *grandParent = node->parent->parent;
        return grandParent ? grandParent->children.count() : m_root->children.count();
    }
    return node->children.count();
}

int VideoModel::columnCount(const QModelIndex &idx) const
{
    Q_UNUSED(idx);
    return 1;
}

bool VideoModel::hasChildren(const QModelIndex &parent) const
{
    Node *parentNode = parent.isValid() ? static_cast<Node *>(parent.internalPointer()) : m_root;
    return parentNode->type != Node::MovieNode;
}

void VideoModel::fetchMoreTopLevel()
{
    m_root->loading = true;
    QString fromTitle;
    QStringList ignoreIds;

    for (int i = m_root->children.count()-1; i >= 0; i--) {
        Node *child = m_root->children[i];

        if (!fromTitle.isEmpty() && fromTitle != child->text)
            break;

        fromTitle = child->text;
        ignoreIds.append(QString::number(child->id));
    }
    QMetaObject::invokeMethod(m_reader, "execute", Qt::QueuedConnection, Q_ARG(QSqlQuery, readMoviesQuery(fromTitle, ignoreIds)), Q_ARG(void *, m_root));
}

bool VideoModel::canFetchMore(const QModelIndex &parent) const
{
    Node *parentNode = parent.isValid() ? static_cast<Node *>(parent.internalPointer()) : m_root;
    return !parentNode->loading && !parentNode->loaded;
}

void VideoModel::fetchMore(const QModelIndex &parent)
{
    if (!canFetchMore(parent))
        return;

    DEBUG << parent;

    ++m_readerResponsePending;
    if (!parent.isValid()) {
        fetchMoreTopLevel();
        return;
    }
}

bool VideoModel::movieLessThan(VideoModel::Node *n1, VideoModel::Node *n2)
{
    // ## Fix this to be in sync with the sqlite sorting!
    return n1->text < n2->text;
}

void VideoModel::handleDataReady(DbReader *reader, const QList<QSqlRecord> &records, void *node)
{
    Q_ASSERT(reader == m_reader);
    DEBUG << "Received response from db of size " << records.size();

    QList<Node *> newChildren;
    QHash<int, Node *> newChildIds;
    Node *loadingNode = static_cast<Node *>(node);

    for (int i = 0; i < records.count(); i++) {
        int id = records[i].value("id").toInt();
        if (loadingNode->childIds.contains(id)) {
            DEBUG << "Ignoring duplicate in query " << id;
            continue;
        }

        Node *node = new Node(loadingNode, Node::MovieNode);
        node->id = id;
        node->text = records[i].value("title").toString();
        node->filePath = records[i].value("filepath").toString();
        node->hasThumbnail = !records[i].value("thumbnail").toByteArray().isEmpty();
        node->loaded = true;

        newChildIds.insert(id, node);
        newChildren.append(node);
    }

    // update these variables, because beginInsertRows() can call canFetchMore
    --m_readerResponsePending;
    loadingNode->loading = false;
    if (records.count() < LIMIT || loadingNode != m_root) { // root node is always incrementatlly fetched
        DEBUG << "Marking node as loaded";
        loadingNode->loaded = true;
    } else {
        DEBUG << "More to come";
    }

    if (!newChildren.isEmpty()) {
        DEBUG << "Adding " << newChildren.count() << " children";
        const int from = loadingNode->children.count();
        beginInsertRows(loadingNode == m_root ? QModelIndex() : createIndex(loadingNode->parent->children.indexOf(loadingNode), 0, loadingNode), 
                        from, from+newChildren.count()-1);
        loadingNode->children.append(newChildren);
        loadingNode->childIds.unite(newChildIds);
        endInsertRows();
    }

    if (!m_readerResponsePending)
        handleDatabaseUpdated(m_databaseUpdatesQueue);
}

QStringList VideoModel::groupByOptions() const
{
    return QStringList() << tr("None");
}

void VideoModel::groupBy(VideoModel::GroupBy groupBy)
{
    beginResetModel();

    m_groupBy = groupBy;

    qDeleteAll(m_root->children);
    m_root->children.clear();
    m_root->childIds.clear();
    m_root->loading = m_root->loaded = false;

    DbReader *newReader = new DbReader;
    if (m_reader) {
        disconnect(m_reader, 0, this, 0);
        m_reader->stop();
        m_reader->deleteLater();
    }
    m_reader = newReader;
    m_readerResponsePending = 0;

    if (!m_readerThread) {
        m_readerThread = new QThread(this);
        m_readerThread->start();
    }
    m_reader->moveToThread(m_readerThread);
    QMetaObject::invokeMethod(m_reader, "initialize", Q_ARG(QSqlDatabase, Backend::instance()->mediaDatabase()));
    connect(m_reader, SIGNAL(dataReady(DbReader *, QList<QSqlRecord>, void *)), 
            this, SLOT(handleDataReady(DbReader *, QList<QSqlRecord>, void *)));

    endResetModel();

    if (canFetchMore(QModelIndex()))
        fetchMore(QModelIndex());
}

void VideoModel::handleDatabaseUpdated(const QList<QSqlRecord> &records)
{
    if (m_readerResponsePending) {
        DEBUG << "Waiting for database. " << m_readerResponsePending << "pending";
        m_databaseUpdatesQueue.append(records);
        return;
    }

    foreach(const QSqlRecord &record, records)
        handleDatabaseUpdated(record);
}

void VideoModel::handleDatabaseUpdated(const QSqlRecord &record)
{
    updateMovie(record, m_root);
}

void VideoModel::updateMovie(const QSqlRecord &record, Node *parentNode)
{
    const int id = record.value("id").toInt();
    qDebug() << "Updating a movie node with id " << id;
    const QString text = record.value("title").toString();
    Node *movieNode = parentNode->childIds.value(id); 
    if (movieNode) {
        // always update. the scanner always has the latest information
        movieNode->hasThumbnail = !record.value("thumbnail").toByteArray().isEmpty();
        int idx = parentNode->children.indexOf(movieNode);
        if (movieNode->text == text) {
            DEBUG << "Updating existing node with dataChanged";
            emit dataChanged(createIndex(idx, 0, movieNode), createIndex(idx, 0, movieNode));
            return;
        }

        DEBUG << "Moving existing node";

        beginRemoveRows(indexForNode(parentNode), idx, idx);
        movieNode->children.removeAt(idx);
        endRemoveRows();
    } else {
        if (!parentNode->loaded && (parentNode->children.isEmpty() || text > parentNode->children.last()->text)) { // ## wrong, must use same compare as sqlite
            DEBUG << "Out of bounds";
            return; // some random movie, out of our bounds
        }

        Node *newNode = new Node(parentNode, Node::MovieNode);
        newNode->id = id;
        newNode->text = text;
        newNode->filePath = record.value("filepath").toString();
        newNode->loaded = true;
        newNode->hasThumbnail = !record.value("thumbnail").toByteArray().isEmpty();
        movieNode = newNode;
    }

    QList<Node *>::iterator it = qLowerBound(parentNode->children.begin(), parentNode->children.end(), movieNode, movieLessThan);
    int idx = it - parentNode->children.begin();
    //DEBUG << "Inserting new node at " << idx;
    beginInsertRows(indexForNode(parentNode), idx, idx);
    parentNode->children.insert(it, movieNode);
    parentNode->childIds.insert(movieNode->id, movieNode);
    endInsertRows();
}

VideoModelImageProvider::VideoModelImageProvider()
    : QDeclarativeImageProvider(QDeclarativeImageProvider::Image)
{
}

VideoModelImageProvider::~VideoModelImageProvider()
{
}

QImage VideoModelImageProvider::requestImage(const QString &id, QSize *size, const QSize &requestedSize)
{
    QSqlDatabase db = QSqlDatabase::cloneDatabase(Backend::instance()->mediaDatabase(), QUuid::createUuid().toString());
    db.open();
    QSqlQuery query(db);
    query.prepare("SELECT thumbnail FROM video WHERE id = :id");
    query.bindValue(":id", id.mid(6).toInt());
    query.exec();

    if (!query.next()) {
        DEBUG << "No such id!" << id.mid(6).toInt() << id;
        return QImage();
    }
    QByteArray ba = query.value(0).toByteArray();
    QImage img;
    if (ba.isEmpty()) {
        img = QImage("defaultvideo.png"); // ## Actually create this
    } else if (ba.startsWith("file://")) {
        img = QImage(QFile::decodeName(ba.mid(7)));
    } else {
        img = QImage::fromData(ba);
        if (requestedSize.isValid())
            img = img.scaled(requestedSize);
    }
    if (size)
        *size = img.size();
    return img;
}

void VideoModel::setThemeResourcePath(const QString &themePath)
{
    qDebug() << "Set the theme path to " << themePath;
}

void VideoModel::addSearchPath(const QString &path, const QString &name)
{
    QMetaObject::invokeMethod(MediaScanner::instance(), "addSearchPath", Qt::QueuedConnection, Q_ARG(QString, "video"), Q_ARG(QString, path), Q_ARG(QString, name));
}

void VideoModel::removeSearchPath(int index)
{
}

