#ifndef MEDIAMODEL_H
#define MEDIAMODEL_H

#include <QtCore>
#include <QtGui>
#include <QtDeclarative>
#include <QtSql>

class QThread;
class MediaScanner;
class DbReader;

class MediaModel : public QAbstractItemModel
{
    Q_OBJECT
    Q_PROPERTY(int depth READ depth NOTIFY depthChanged)
    Q_PROPERTY(QString structure READ structure WRITE setStructure NOTIFY structureChanged)

public:
    MediaModel(QObject *parent = 0);
    ~MediaModel();

    // callable from QML
    Q_INVOKABLE void addSearchPath(const QString &mediaPath, const QString &name);
    Q_INVOKABLE void removeSearchPath(int index);

    QString structure() const;
    void setStructure(const QString &str);

    int depth() const;
    Q_INVOKABLE void back();
    Q_INVOKABLE void enter(int index);

    // reimp
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QModelIndex index(int row, int col, const QModelIndex &parent) const;
    QModelIndex parent(const QModelIndex &idx) const;
    Q_INVOKABLE int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &idx = QModelIndex()) const;
    bool hasChildren(const QModelIndex &parent = QModelIndex()) const;
    bool canFetchMore(const QModelIndex &parent) const;
    void fetchMore(const QModelIndex &parent);

signals:
    void depthChanged();
    void structureChanged();

private slots:
    void handleDataReady(DbReader *reader, const QList<QSqlRecord> &data, void *node);
    void handleDatabaseUpdated(const QList<QSqlRecord> &record);

private:
    void initialize();
    QSqlQuery query();

    int m_depth;
    QString m_structure;
    QList<QHash<QString, QVariant> > m_data;
    bool m_loading, m_loaded;

    QList<QHash<QString, QVariant> > m_cursor;

    DbReader *m_reader;
    QThread *m_readerThread;
    QString m_mediaType;
};

#endif // MEDIAMODEL_H

