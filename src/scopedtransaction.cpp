#include "scopedtransaction.h"
#include <QtDebug>
#include <QFile>
#include <QStringList>
#include <QSqlError>

ScopedTransaction::ScopedTransaction(QSqlDatabase &db) 
    : m_db(db), m_query(db), m_errored(false) 
{ 
    m_db.transaction(); 
}

ScopedTransaction::~ScopedTransaction() 
{
    if (!m_errored)
        m_db.commit(); 
}

bool ScopedTransaction::exec(const QString &query) 
{
    if (m_errored) {
        qDebug() << query << " skipped, transaction already errored";
        return false;
    }

    if (!m_query.exec(query)) {
        qDebug() << query << " failed with error : " << m_query.lastError();
        m_db.rollback();
        m_errored = true;
        return false;
    }
    return true;
}

bool ScopedTransaction::execFile(const QString &fileName) 
{
    if (m_errored) {
        qDebug() << fileName << " skipped, transaction already errored";
        return false;
    }

    QFile file(fileName);
    if (!file.open(QFile::ReadOnly)) {
        qDebug() << fileName << " could not be opened for exec";
        return false;
    }

    // Need to split the queries by hand - QTBUG-8689
    QString fileContents = file.readAll();
    QStringList queries = fileContents.split(";\n\n", QString::SkipEmptyParts);
    foreach(const QString &query, queries) {
        if (!m_query.exec(query)) {
            qDebug() << fileName << " failed to exec " << query << " . Error : " << m_query.lastError();
            m_db.rollback();
            m_errored = true;
            return false;
        }
    }

    return true;
}

