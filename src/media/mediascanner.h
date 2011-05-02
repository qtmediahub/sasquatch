#ifndef MEDIASCANNER_H
#define MEDIASCANNER_H

#include <QObject>
#include <QFileInfo>
#include <QSqlDatabase>
#include <QSqlRecord>
#include <QHash>

class MediaParser;

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

    void addParser(MediaParser *);

    void stop() { m_stop = true; }

public slots:
    void addSearchPath(const QString &type, const QString &path, const QString &name);
    void refresh();
    
signals:
    void databaseUpdated(const QList<QSqlRecord> &records);

private:
    void scan(MediaParser *parser, const QString &path);

    volatile bool m_stop;
    QSqlDatabase m_db;
    QString m_errorString;

    QHash<QString, MediaParser *> m_parsers;
};

#endif // MEDIASCANNER_H

