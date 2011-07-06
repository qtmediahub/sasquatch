#ifndef MEDIASCANNER_H
#define MEDIASCANNER_H

#include <QObject>
#include <QFileInfo>
#include <QSqlDatabase>
#include <QSqlRecord>
#include <QHash>
#include "global.h"

class MediaParser;

// MediaScanner is designed to be run in a separate thread. Do not call the methods
// below directly from the ui thread (except addParser())!
class QMH_EXPORT MediaScanner : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString currentScanPath READ currentScanPath NOTIFY currentScanPathChanged)

public:
    static MediaScanner *instance();
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

    QString thumbnailPath() const;

    QString currentScanPath() const { return m_currentScanPath; }

public slots:
    void initialize();
    void addSearchPath(const QString &type, const QString &path, const QString &name);
    void refresh(const QString &type = QString());
    
signals:
    void currentScanPathChanged();

private:
    MediaScanner(QObject *parent = 0);
    void scan(MediaParser *parser, const QString &path);

    static MediaScanner *s_instance;
    volatile bool m_stop;
    QSqlDatabase m_db;
    QString m_errorString;

    QString m_currentScanPath;

    QHash<QString, MediaParser *> m_parsers;
};

#endif // MEDIASCANNER_H

