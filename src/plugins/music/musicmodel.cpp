#include "musicmodel.h"
#include "musicparser.h"
#include "mediascanner.h"
#include "scopedtransaction.h"
#include "backend.h"
#include "dbreader.h"
#include <QtSql>
#include "media.h"

#define DEBUG if (1) qDebug() << __PRETTY_FUNCTION__

static const int LIMIT = 200;

static QSqlQuery readSongsQuery(const QString &lastTitle, const QStringList &ignoreIds)
{
    DEBUG << "Reading songs from " << lastTitle;

    QSqlQuery query(Backend::instance()->mediaDatabase());
    query.setForwardOnly(true);
    if (!ignoreIds.isEmpty()) {
        // This query will result in duplicates because of >=. But it's necessary to ensure that we retreive all songs with the same
        // title and we were split up midway.
        // If you change the query here, change the scanner code also
        query.prepare(QString("SELECT id, filepath, title, album, thumbnail FROM music WHERE title >= :title AND id NOT IN (%1) ORDER BY title LIMIT :limit").arg(ignoreIds.join(",")));
        query.bindValue(":title", lastTitle);
    } else {
        query.prepare("SELECT id, filepath, title, album, thumbnail FROM music ORDER BY title LIMIT :limit");
    }
    query.bindValue(":limit", LIMIT);
    return query;
}

static QSqlQuery readAlbumsQuery(const QString &lastAlbum, const QString &lastArtist)
{
    DEBUG << "Reading albums";

    QSqlQuery query(Backend::instance()->mediaDatabase());
    query.setForwardOnly(true);
    if (!lastAlbum.isEmpty()) {
        query.prepare("SELECT album, artist FROM music GROUP BY album, artist HAVING album >= :album AND artist >= :artist "
                      "ORDER BY album, artist LIMIT :limit OFFSET 1");
        query.bindValue(":album", lastAlbum);
        query.bindValue(":artist", lastArtist);
    } else {
        query.prepare("SELECT album, artist FROM music GROUP BY album, artist ORDER BY album, artist LIMIT :limit");
    }
    query.bindValue(":limit", LIMIT);
    return query;
}

static QSqlQuery readArtistsQuery(const QString &fromArtist)
{
    DEBUG << "Reading artists";

    QSqlQuery query(Backend::instance()->mediaDatabase());
    query.setForwardOnly(true);
    if (!fromArtist.isEmpty()) {
        query.prepare("SELECT DISTINCT artist FROM music WHERE artist > :artist ORDER BY artist LIMIT :limit");
        query.bindValue(":artist", fromArtist);
    } else {
        query.prepare("SELECT DISTINCT artist FROM music ORDER BY artist LIMIT :limit");
    }
    query.bindValue(":limit", LIMIT);
    return query;
}

static QSqlQuery readSongsOfAlbumQuery(const QString &album, const QString &artist)
{
    DEBUG << "Reading songs of album " << album << " and artist " << artist;

    QSqlQuery query(Backend::instance()->mediaDatabase());
    query.setForwardOnly(true);
    query.prepare("SELECT id, filepath, title, album, thumbnail FROM music WHERE album = :album AND artist = :artist ORDER BY track, title");
    query.bindValue(":album", album);
    query.bindValue(":artist", artist);
    return query;
}

static QSqlQuery readAlbumsOfArtistQuery(const QString &artist)
{
    DEBUG << "Reading albums of artist " << artist;

    QSqlQuery query(Backend::instance()->mediaDatabase());
    query.setForwardOnly(true);
    query.prepare("SELECT DISTINCT album, artist FROM music WHERE artist = :artist ORDER BY album");
    query.bindValue(":artist", artist);
    return query;
}

MusicModel::MusicModel(QObject *parent)
    : QAbstractItemModel(parent), m_root(0), m_reader(0), m_readerThread(0),
      m_groupBy(NoGrouping), m_readerResponsePending(0)
{
    QHash<int, QByteArray> hash = roleNames();
    hash.unite(Media::roleNames());
    setRoleNames(hash);

    MusicParser *parser = new MusicParser;
    MediaScanner::instance()->addParser(parser);
    connect(parser, SIGNAL(databaseUpdated(QList<QSqlRecord>)),
            this, SLOT(handleDatabaseUpdated(QList<QSqlRecord>)));

    m_root = new Node(0, Node::RootNode);

    groupBy(NoGrouping);
}

MusicModel::~MusicModel()
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

QVariant MusicModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    Node *node = static_cast<Node *>(index.internalPointer());

    switch (role) {
    case Qt::DisplayRole: 
        if (node->type == Node::DotDot)
            return node->text;
        if (m_groupBy == NoGrouping)
            return node->text + " (" + node->filePath + ')';
        else if (m_groupBy == GroupByAlbum)
            return node->text + " (" + node->artist + ")"; // ## cache this
        else
            return node->text;
    case Media::PreviewUrlRole: 
        if (node->type == Node::ArtistNode)
            return QString("image://musicmodel/artist/%1").arg(node->artist);
        else if (node->type == Node::AlbumNode)
            return QString("image://musicmodel/album/%1 %2").arg(node->text).arg(node->artist);
        else if (node->type == Node::SongNode) {
            if (!node->hasThumbnail)
                return QVariant();
            return QString("image://musicmodel/song/%1").arg(node->id);
        }
        return QVariant();
    case Media::FilePathRole:
        return node->filePath;
    default: 
        return QVariant();
    }
}

QModelIndex MusicModel::index(int row, int col, const QModelIndex &parent) const
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

QModelIndex MusicModel::indexForNode(MusicModel::Node *node) const
{
    return node == m_root
            ? QModelIndex()
            : createIndex(node->parent->children.indexOf(node), 0, node);
}

QModelIndex MusicModel::parent(const QModelIndex &idx) const
{
    if (!idx.isValid())
        return QModelIndex();
    Node *node = static_cast<Node *>(idx.internalPointer());
    Node *parent = node->parent;
    Node *grandParent = parent && parent->parent ? parent->parent : m_root;
    return createIndex(grandParent->children.indexOf(parent), idx.column(), parent);
}

int MusicModel::rowCount(const QModelIndex &parent) const
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

int MusicModel::columnCount(const QModelIndex &idx) const
{
    Q_UNUSED(idx);
    return 1;
}

bool MusicModel::hasChildren(const QModelIndex &parent) const
{
    Node *parentNode = parent.isValid() ? static_cast<Node *>(parent.internalPointer()) : m_root;
    return parentNode->type != Node::SongNode;
}

void MusicModel::fetchMoreTopLevel()
{
    m_root->loading = true;
    if (m_groupBy == NoGrouping) {
        QString fromTitle;
        QStringList ignoreIds;

        for (int i = m_root->children.count()-1; i >= 0; i--) {
            Node *child = m_root->children[i];

            if (!fromTitle.isEmpty() && fromTitle != child->text)
                break;

            fromTitle = child->text;
            ignoreIds.append(QString::number(child->id));
        }
        QMetaObject::invokeMethod(m_reader, "execute", Qt::QueuedConnection, Q_ARG(QSqlQuery, readSongsQuery(fromTitle, ignoreIds)), Q_ARG(void *, m_root));
    } else if (m_groupBy == GroupByAlbum) {
        QString fromAlbum = m_root->children.isEmpty() ? QString() : m_root->children.last()->text;
        QString fromArtist = m_root->children.isEmpty() ? QString() : m_root->children.last()->artist;
        QMetaObject::invokeMethod(m_reader, "execute", Qt::QueuedConnection, Q_ARG(QSqlQuery, readAlbumsQuery(fromAlbum, fromArtist)), Q_ARG(void *, m_root));
    } else if (m_groupBy == GroupByArtist) {
        QString fromArtist = m_root->children.isEmpty() ? QString() : m_root->children.last()->text;
        QMetaObject::invokeMethod(m_reader, "execute", Qt::QueuedConnection, Q_ARG(QSqlQuery, readArtistsQuery(fromArtist)), Q_ARG(void *, m_root));
    }
}

bool MusicModel::canFetchMore(const QModelIndex &parent) const
{
    Node *parentNode = parent.isValid() ? static_cast<Node *>(parent.internalPointer()) : m_root;
    return !parentNode->loading && !parentNode->loaded;
}

void MusicModel::fetchMore(const QModelIndex &parent)
{
    if (!canFetchMore(parent))
        return;

    DEBUG << parent;

    ++m_readerResponsePending;
    if (!parent.isValid()) {
        fetchMoreTopLevel();
        return;
    }

    Node *node = static_cast<Node *>(parent.internalPointer());
    node->loading = true;
    if (node->type == Node::AlbumNode) {
        QMetaObject::invokeMethod(m_reader, "execute", Qt::QueuedConnection, Q_ARG(QSqlQuery, readSongsOfAlbumQuery(node->text, node->artist)), Q_ARG(void *, node));
    } else if (node->type == Node::ArtistNode) {
        QMetaObject::invokeMethod(m_reader, "execute", Qt::QueuedConnection, Q_ARG(QSqlQuery, readAlbumsOfArtistQuery(node->text)), Q_ARG(void *, node));
    }
}

bool MusicModel::nodeLessThan(MusicModel::Node *n1, MusicModel::Node *n2)
{
    // ## Fix this to be in sync with the sqlite sorting!
    return n1->text < n2->text;
}

bool MusicModel::artistLessThan(MusicModel::Node *n1, MusicModel::Node *n2)
{
    // ## Fix this to be in sync with the sqlite sorting!
    return n1->artist < n2->artist;
}

bool MusicModel::albumLessThan(MusicModel::Node *n1, MusicModel::Node *n2)
{
    // ## Fix this to be in sync with the sqlite sorting!
    if (n1->text == n2->text)
        return n1->artist < n2->artist;
    return n1->text < n2->text;
}

void MusicModel::handleDataReady(DbReader *reader, const QList<QSqlRecord> &records, void *node)
{
    Q_ASSERT(reader == m_reader);
    DEBUG << "Received response from db of size " << records.size();

    QList<Node *> newChildren;
    QHash<int, Node *> newChildIds;
    Node *loadingNode = static_cast<Node *>(node);

    const bool loadingSong = m_groupBy == NoGrouping || loadingNode->type == Node::AlbumNode;

    for (int i = 0; i < records.count(); i++) {
        int id = records[i].value("id").toInt();
        if (loadingSong && loadingNode->childIds.contains(id)) {
            DEBUG << "Ignoring duplicate in query " << id;
            continue;
        }

        Node *node;
        if (m_groupBy == NoGrouping || loadingNode->type == Node::AlbumNode) {
            node = new Node(loadingNode, Node::SongNode);
            node->id = id;
            node->text = records[i].value("title").toString();
            node->filePath = records[i].value("filepath").toString();
            node->hasThumbnail = !records[i].value("thumbnail").toByteArray().isEmpty();
            node->loaded = true;
        } else if (m_groupBy == GroupByAlbum || loadingNode->type == Node::ArtistNode) {
            node = new Node(loadingNode, Node::AlbumNode);
            node->text = records[i].value("album").toString();
            node->artist = records[i].value("artist").toString();
        } else { // GroupByArtist
            node = new Node(loadingNode, Node::ArtistNode);
            node->text = records[i].value("artist").toString();
        }

        if (loadingSong)
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

QStringList MusicModel::groupByOptions() const
{
    return QStringList() << tr("None") << tr("Artist") << tr("Album");
}

void MusicModel::groupBy(MusicModel::GroupBy groupBy)
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

void MusicModel::handleDatabaseUpdated(const QList<QSqlRecord> &records)
{
    if (m_readerResponsePending) {
        DEBUG << "Waiting for database. " << m_readerResponsePending << "pending";
        m_databaseUpdatesQueue.append(records);
        return;
    }

    foreach(const QSqlRecord &record, records)
        handleDatabaseUpdated(record);
}

void MusicModel::handleDatabaseUpdated(const QSqlRecord &record)
{
    DEBUG << m_groupBy;
    if (m_groupBy == NoGrouping) {
        updateSong(record, m_root);
    } else if (m_groupBy == GroupByAlbum) {
        if (Node *albumNode = updateAlbum(record, m_root))
            updateSong(record, albumNode);
    } else { // GroupByArtist
        if (Node *artistNode = updateArtist(record, m_root))
            if (Node *albumNode = updateAlbum(record, artistNode))
                updateSong(record, albumNode);
    }
}

MusicModel::Node *MusicModel::updateArtist(const QSqlRecord &record, MusicModel::Node *parentNode)
{
    QString artist = record.value("artist").toString();

    //DEBUG << "Searching for " << artist;

    Node node(0, Node::ArtistNode);
    node.artist = artist;
    Node *artistNode;
    QList<Node *>::iterator it = qLowerBound(parentNode->children.begin(), parentNode->children.end(), &node, artistLessThan);
    if (it == parentNode->children.end() || (*it)->artist != artist) {
        if (!parentNode->loaded && (parentNode->children.isEmpty() || artist > parentNode->children.last()->artist)) { // ## wrong, must use same compare as sqlite
            DEBUG << "Out of bounds";
            return 0; // some random artist, out of our bounds
        }

        Node *newNode = new Node(parentNode, Node::ArtistNode);
        newNode->text = artist;
        newNode->artist = artist;
        int idx = it - parentNode->children.begin();
        beginInsertRows(indexForNode(parentNode), idx, idx);
        parentNode->children.insert(it, newNode);
        endInsertRows();

        artistNode = newNode;
    } else {
        artistNode = *it;
    }

    return artistNode;
}

MusicModel::Node *MusicModel::updateAlbum(const QSqlRecord &record, MusicModel::Node *parentNode)
{
    QString album = record.value("album").toString();
    QString artist = record.value("artist").toString();

    //DEBUG << "Updating album " << album << artist;

    Node node(0, Node::AlbumNode);
    node.text = album;
    node.artist = artist;
    Node *albumNode;
    QList<Node *>::iterator it = qLowerBound(parentNode->children.begin(), parentNode->children.end(), &node, albumLessThan);
    if (it == parentNode->children.end() || !((*it)->text == album && (*it)->artist == artist)) {
        if (!parentNode->loaded) {
            if (parentNode->children.isEmpty()) {
                DEBUG << "Out of bounds";
                return 0;
            }
            Node *lastChild = parentNode->children.last();
            if (album > lastChild->text || (album == lastChild->text && artist > lastChild->artist)) { // ## wrong, must use same compare as sqlite
                DEBUG << "Out of bounds";
                return 0; // some album, out of our bounds
            }
        }

        Node *newNode = new Node(parentNode, Node::AlbumNode);
        newNode->text = album;
        newNode->artist = artist;
        int idx = it - parentNode->children.begin();
        beginInsertRows(indexForNode(parentNode), idx, idx);
        parentNode->children.insert(it, newNode);
        endInsertRows();

        albumNode = newNode;
    } else {
        albumNode = *it;
    }

    return albumNode;
}

void MusicModel::updateSong(const QSqlRecord &record, Node *parentNode)
{
    const int id = record.value("id").toInt();
    const QString text = record.value("title").toString();
    Node *songNode = parentNode->childIds.value(id); 
    if (songNode) {
        // always update. the scanner always has the latest information
        songNode->hasThumbnail = !record.value("thumbnail").toByteArray().isEmpty();
        int idx = parentNode->children.indexOf(songNode);
        if (songNode->text == text) {
            DEBUG << "Updating existing node with dataChanged";
            emit dataChanged(createIndex(idx, 0, songNode), createIndex(idx, 0, songNode));
            return;
        }

        DEBUG << "Moving existing node";

        beginRemoveRows(indexForNode(parentNode), idx, idx);
        songNode->children.removeAt(idx);
        endRemoveRows();
    } else {
        if (!parentNode->loaded && (parentNode->children.isEmpty() || text > parentNode->children.last()->text)) { // ## wrong, must use same compare as sqlite
            DEBUG << "Out of bounds";
            return; // some random song, out of our bounds
        }

        Node *newNode = new Node(parentNode, Node::SongNode);
        newNode->id = id;
        newNode->text = text;
        newNode->filePath = record.value("filepath").toString();
        newNode->loaded = true;
        newNode->hasThumbnail = !record.value("thumbnail").toByteArray().isEmpty();
        songNode = newNode;
    }

    QList<Node *>::iterator it = qLowerBound(parentNode->children.begin(), parentNode->children.end(), songNode, nodeLessThan);
    int idx = it - parentNode->children.begin();
    //DEBUG << "Inserting new node at " << idx;
    beginInsertRows(indexForNode(parentNode), idx, idx);
    parentNode->children.insert(it, songNode);
    endInsertRows();
}

MusicModelImageProvider::MusicModelImageProvider()
    : QDeclarativeImageProvider(QDeclarativeImageProvider::Image)
{
}

MusicModelImageProvider::~MusicModelImageProvider()
{
}

QImage MusicModelImageProvider::requestImage(const QString &id, QSize *size, const QSize &requestedSize)
{
    QSqlDatabase db = QSqlDatabase::cloneDatabase(Backend::instance()->mediaDatabase(), QUuid::createUuid().toString());
    db.open();
    QSqlQuery query(db);
    query.prepare("SELECT thumbnail FROM music WHERE id = :id");
    query.bindValue(":id", id.mid(5).toInt());
    query.exec();

    if (!query.next()) {
        DEBUG << "No such id!" << id.mid(5).toInt();
        return QImage();
    }
    QByteArray ba = query.value(0).toByteArray();
    QImage img;
    if (ba.isEmpty()) {
        img = QImage("defaultmusic.png"); // ## Actually create this
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

void MusicModel::setThemeResourcePath(const QString &themePath)
{
    qDebug() << "Set the theme path to " << themePath;
}

void MusicModel::addSearchPath(const QString &path, const QString &name)
{
    QMetaObject::invokeMethod(MediaScanner::instance(), "addSearchPath", Qt::QueuedConnection, Q_ARG(QString, "music"), Q_ARG(QString, path), Q_ARG(QString, name));
}

void MusicModel::removeSearchPath(int index)
{
    Q_UNUSED(index);
}

