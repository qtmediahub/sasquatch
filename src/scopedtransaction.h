#ifndef SCOPEDTRANSACTION_H
#define SCOPEDTRANSACTION_H

#include <QSqlDatabase>
#include <QSqlQuery>

class ScopedTransaction
{
public:
    ScopedTransaction(QSqlDatabase &db);
    ~ScopedTransaction();

    bool exec(const QString &query);
    bool execFile(const QString &fileName);

private:
    QSqlDatabase &m_db;
    QSqlQuery m_query;
    bool m_errored;
};

#endif // SCOPEDTRANSACTION_H

