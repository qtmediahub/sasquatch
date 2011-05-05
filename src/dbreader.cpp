#include "dbreader.h"

#define DEBUG if (0) qDebug() << __PRETTY_FUNCTION__

DbReader::DbReader(QObject *parent)
    : QObject(parent), m_stop(false)
{
    qRegisterMetaType<QList<QSqlRecord> >();
    qRegisterMetaType<QSqlDatabase>();
    qRegisterMetaType<QSqlQuery>();
    qRegisterMetaType<DbReader *>();
    qRegisterMetaType<void *>();
}

DbReader::~DbReader()
{
    QSqlDatabase::removeDatabase(m_db.connectionName());
}

void DbReader::stop() 
{
    m_stop = true;
}

void DbReader::initialize(const QSqlDatabase &db)
{
    m_db = QSqlDatabase::cloneDatabase(db, QUuid::createUuid().toString());
    if (!m_db.open())
        qFatal("Erorr opening database: %s", qPrintable(m_db.lastError().text()));
}

void DbReader::execute(const QSqlQuery &q, void *userData)
{
    QSqlQuery query(q);
    query.setForwardOnly(true);
    query.exec();

    QList<QSqlRecord> data = readRecords(query);
    DEBUG << "Read " << data.count() << "records";

    if (!m_stop)
        emit dataReady(this, data, userData);
}

QList<QSqlRecord> DbReader::readRecords(QSqlQuery &query)
{
    QList<QSqlRecord> data;
    while (query.next() && !m_stop) {
        data.append(query.record());
    }
    return data;
}

