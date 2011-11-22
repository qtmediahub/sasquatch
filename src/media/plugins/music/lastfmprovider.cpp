#include "lastfmprovider.h"

#define DEBUG if (0) qDebug() << __PRETTY_FUNCTION__

LastFMProvider::LastFMProvider(QSqlDatabase db, QObject *parent) :
    QObject(parent),
    m_db(db)
{
    m_networkAccessManager = new QNetworkAccessManager(this);

    m_query = new QSqlQuery(m_db);
    m_query->setForwardOnly(true);
    m_query->exec("SELECT id, artist, title, album FROM music WHERE thumbnail IS NULL");

    connect(this, SIGNAL(replyFinished()), this, SLOT(requestNext()));
    emit replyFinished();
}

void LastFMProvider::handleReply()
{
    QXmlStreamReader reader(m_reply);

    QString artist;
    QString title;
    QString album;
    QString track;
    QString thumbnail;

    bool trackTagFound = false;
    bool trackNameTagFound = false;
    bool albumTagFound = false;
    bool albumTitleTagFound = false;
    bool artistTagFound = false;
    bool artistNameTagFound = false;

    while (reader.readNext() && !reader.isEndDocument()) {
        if (reader.isStartElement() && reader.name() == "track") {
            trackTagFound = true;
        }

        if (trackTagFound && !trackNameTagFound && reader.isStartElement() && reader.name() == "name") {
            trackNameTagFound = true;
            if (reader.readNext() && reader.isCharacters())
                title = reader.text().toString();
        }

        if (trackTagFound && reader.isStartElement() && reader.name() == "artist") {
            artistTagFound = true;
        }

        if (artistTagFound && !artistNameTagFound && reader.isStartElement() && reader.name() == "name") {
            artistNameTagFound = true;
            if (reader.readNext() && reader.isCharacters())
                artist = reader.text().toString();
        }

        if (trackTagFound && reader.isStartElement() && reader.name() == "album") {
            albumTagFound = true;
            track = reader.attributes().value("position").toString();
        }

        if (albumTagFound && !albumTitleTagFound && reader.isStartElement() && reader.name() == "title") {
            artistNameTagFound = true;
            if (reader.readNext() && reader.isCharacters())
                album = reader.text().toString();
        }

        if (albumTagFound && reader.isStartElement() && reader.name() == "image" && reader.attributes().value("size") == "large") {
            if (reader.readNext() && reader.isCharacters())
                thumbnail = reader.text().toString();
        }
    }

    DEBUG << "metadata from lastfm:" << title << artist << album << track << thumbnail;

    QSqlQuery query(m_db);
    if (!query.prepare("UPDATE music SET thumbnail=:thumbnail WHERE id=:id")) {
        qWarning() << query.lastError().text();
    }

    query.bindValue(":thumbnail", thumbnail);
    query.bindValue(":id", m_currentId);

    if (!query.exec())
        qWarning() << query.lastError().text();

    emit replyFinished();
}

void LastFMProvider::requestNext()
{
    if (m_query->next()) {
        const QString id = m_query->value(0).toString();
        const QString artist = m_query->value(1).toString();
        const QString title = m_query->value(2).toString();
        const QString album = m_query->value(3).toString();

        DEBUG << "empty thumbnail for" << id << artist << title << album;

        static const QString apiUri = QString::fromLatin1("http://ws.audioscrobbler.com/2.0/?method=track.getinfo&api_key=e7e3601ddce49a72b56c758d29af4a3f&");

        QString requestString = apiUri + "artist=" + artist + "&track=" + title;

        DEBUG << "query" << requestString;

        m_currentId = id;

        m_reply = m_networkAccessManager->get(QNetworkRequest(QUrl(requestString)));
        connect(m_reply, SIGNAL(readyRead()), this, SLOT(handleReply()));
    } else {
        emit allRequestsFinished();
    }
}
