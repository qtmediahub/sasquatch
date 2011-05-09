#ifndef MEDIASCANNER_H
#define MEDIASCANNER_H

#include <QObject>
#include <QFileInfo>
#include <QSqlDatabase>
#include <QSqlRecord>
#include <QHash>
#include "global.h"

class MediaParser;

class QMH_EXPORT MediaScanner : public QObject
{
    Q_OBJECT
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

public slots:
    void initialize();
    void addSearchPath(const QString &type, const QString &path, const QString &name);
    void refresh();
    
private:
    MediaScanner(QObject *parent = 0);
    void scan(MediaParser *parser, const QString &path);

    static MediaScanner *s_instance;
    volatile bool m_stop;
    QSqlDatabase m_db;
    QString m_errorString;

    QHash<QString, MediaParser *> m_parsers;
};

#endif // MEDIASCANNER_H

