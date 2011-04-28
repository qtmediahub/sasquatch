#ifndef MEDIASCANNER_H
#define MEDIASCANNER_H

#include <QObject>
#include <QFileInfo>
#include <QSqlDatabase>
#include <QSqlRecord>
#include <QHash>

class MediaScanner : public QObject
{
    Q_OBJECT
public:
    MediaScanner(const QSqlDatabase &db, QObject *parent = 0);
    ~MediaScanner();

    struct FileInfo {
        QString name;
        quint32 mtime;
        quint32 ctime;
        qint64 size;

        bool valid() const { return !name.isEmpty(); }
    };

    QHash<QString, FileInfo> findFilesByPath(const QString &path);

    void stop() { m_stop = true; }

public slots:
    void addSearchPath(const QString &path, const QString &name);
    void refresh();
    
signals:
    void databaseUpdated(const QList<QSqlRecord> &records);

private:
    void scan(const QString &path);
    void updateMediaInfos(const QList<QFileInfo> &fi);

    volatile bool m_stop;
    QSqlDatabase m_db;
    QString m_errorString;
};

#endif // MEDIASCANNER_H

