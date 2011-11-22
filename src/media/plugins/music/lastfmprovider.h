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
    void handleTitleReply();
    void handleArtistReply();
    void updateDatabase();
    void requestNext();
    void handleError(QNetworkReply::NetworkError code);

private:
    void requestTitleInformation();
    void requestArtistInformation();

    QNetworkAccessManager *m_networkAccessManager;
    QNetworkReply* m_reply;
    QSqlDatabase m_db;
    QSqlQuery *m_query;
    QString m_currentId;
    QString m_currentArtist;
    QString m_currentTitle;
    QString m_currentAlbum;
    QString m_currentThumbnail;
    int m_currentTrack;
};

#endif // LASTFMPROVIDER_H
