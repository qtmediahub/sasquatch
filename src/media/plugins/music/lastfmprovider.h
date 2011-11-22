#ifndef LASTFMPROVIDER_H
#define LASTFMPROVIDER_H

#include <QObject>
#include <QtNetwork>
#include <QtSql>

class LastFMProvider : public QObject
{
    Q_OBJECT
public:
    LastFMProvider(QSqlDatabase db, QObject *parent = 0);

signals:
    void replyFinished();
    void allRequestsFinished();

private slots:
    void handleReply();
    void requestNext();
    void handleError(QNetworkReply::NetworkError code);

private:
    QNetworkAccessManager *m_networkAccessManager;
    QNetworkReply* m_reply;
    QSqlDatabase m_db;
    QSqlQuery *m_query;
    QString m_currentId;
};

#endif // LASTFMPROVIDER_H
