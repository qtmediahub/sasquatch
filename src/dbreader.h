#ifndef MEDIADBREADER_H
#define MEDIADBREADER_H

#include <QtGui>
#include <QtSql>

#include "global.h"

class QMH_EXPORT DbReader : public QObject
{
    Q_OBJECT

public:
    DbReader(QObject *parent = 0);
    ~DbReader();

    void stop();

public slots:
    void initialize(const QSqlDatabase &db);
    void execute(const QSqlQuery &q, void *userData = 0);
    QList<QSqlRecord> readRecords(QSqlQuery &query);

signals:
    void dataReady(DbReader *reader, const QList<QSqlRecord> &data, void *userData);

private:
    QSqlDatabase m_db;
    volatile bool m_stop;
};

Q_DECLARE_METATYPE(QList<QSqlRecord>);
Q_DECLARE_METATYPE(QSqlDatabase);
Q_DECLARE_METATYPE(QSqlQuery);
Q_DECLARE_METATYPE(DbReader *);
Q_DECLARE_METATYPE(void *);

#endif // MEDIADBREADER_H

