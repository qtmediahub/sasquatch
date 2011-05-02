#ifndef MUSICMODEL_H
#define MUSICMODEL_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QAbstractItemModel>
#include <QStringList>
#include <QPixmap>
#include <QSet>
#include <QQueue>
#include <QDeclarativeImageProvider>

#include <QtDeclarative>

class QThread;
class MediaScanner;
class DbReader;

class MusicModel : public QAbstractItemModel
{
    Q_OBJECT
    Q_PROPERTY(QString currentScanPath READ currentScanPath NOTIFY currentScanPathChanged)
    Q_ENUMS(GroupBy)

public:
    MusicModel(QObject *parent = 0);
    ~MusicModel();

    enum GroupBy {
        NoGrouping,
        GroupByArtist,
        GroupByAlbum
    };
    Q_INVOKABLE void groupBy(GroupBy gr);
    Q_INVOKABLE QStringList groupByOptions() const;

    enum {
        PreviewUrlRole = Qt::UserRole +1 
    };

    // callable from QML
    Q_INVOKABLE void setThemeResourcePath(const QString &themePath);
    Q_INVOKABLE void addSearchPath(const QString &mediaPath, const QString &name);
    Q_INVOKABLE void removeSearchPath(int index);

    // reimp
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QModelIndex index(int row, int col, const QModelIndex &parent) const;
    QModelIndex parent(const QModelIndex &idx) const;
    Q_INVOKABLE int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &idx = QModelIndex()) const;
    bool hasChildren(const QModelIndex &parent = QModelIndex()) const;
    bool canFetchMore(const QModelIndex &parent) const;
    void fetchMore(const QModelIndex &parent);

    // has to be public for Q_DECLARE_METATYPE
    struct Node {
        enum Type { RootNode, SongNode, AlbumNode, ArtistNode, DotDot };
        Node(Node *parent, Type type) : type(type), id(0), loaded(false), loading(false), hasThumbnail(false), parent(parent) {
            if (type == AlbumNode || type == ArtistNode) {
                Node *dotDot = new Node(this, DotDot);
                dotDot->text = "..";
                children.append(dotDot);
            }
        }
        ~Node() { qDeleteAll(children); }

        Type type;
        int id;

        QString text;

        // Song nodes
        QString filePath;
        
        // Album nodes
        QString artist;

        bool loaded, loading; // ## Make this enum
        bool hasThumbnail;

        Node *parent;

        QHash<int, Node *> childIds;
        QList<Node *> children;
    };
    static bool nodeLessThan(Node *n1, Node *n2);
    static bool artistLessThan(Node *n1, Node *n2);
    static bool albumLessThan(Node *n1, Node *n2);

    QString currentScanPath() const { return QString(); }

signals:
    void currentScanPathChanged();

private slots:
    void handleDataReady(DbReader *reader, const QList<QSqlRecord> &data, void *node);
    void handleDatabaseUpdated(const QList<QSqlRecord> &record);

private:
    void handleDatabaseUpdated(const QSqlRecord &record);
    void fetchMoreTopLevel();

    Node *updateArtist(const QSqlRecord &record, Node *parentNode);
    Node *updateAlbum(const QSqlRecord &record, Node *parentNode);
    void updateSong(const QSqlRecord &record, Node *parentNode);

    QModelIndex indexForNode(MusicModel::Node *node) const;

    Node *m_root;
    DbReader *m_reader;
    QThread *m_readerThread;
    QQueue<QSqlRecord> m_databaseUpdatesQueue;

    GroupBy m_groupBy;
    int m_readerResponsePending;
};

class MusicModelImageProvider : public QDeclarativeImageProvider
{
public:
    MusicModelImageProvider();
    ~MusicModelImageProvider();

    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize);
};

QML_DECLARE_TYPE(MusicModel)

#endif // MUSICMODEL_H

