#include "client.h"

#include <QtSql>

#include "backend.h"
#include "httpserver.h"

Client::Client(int sockfd, HttpServer *server, QObject *parent) :
    QObject(parent)
{
    m_server = server;
    m_socket = new QTcpSocket(this);
    if (!m_socket->setSocketDescriptor(sockfd)) {
        emit error(m_socket->error());
        return;
    }

    connect(m_socket, SIGNAL(readyRead()), this, SLOT(readClient()));
    connect(m_socket, SIGNAL(disconnected()), this, SLOT(discardClient()));
}

void Client::readClient()
{
    qDebug() << "readClient()";

    if (m_socket->canReadLine()) {
        QByteArray bytesRead = m_socket->readLine();
        QStringList tokens = QString(bytesRead).split(QRegExp("[ \r\n][ \r\n]*"));

        while(m_socket->canReadLine()) {
            bytesRead = m_socket->readLine();
            QList<QByteArray> lineTokens = bytesRead.split(':');
            if (lineTokens.size() == 2) {
                m_request.insert(lineTokens[0], lineTokens[1].trimmed().replace("\r\n",""));
            }
            qDebug() << m_request;
        }

        if (tokens[0] == "GET") {
            m_get = tokens[1];
            qDebug() << "GET" << m_get;
            if (m_get.startsWith("/video"))
                readVideoRequest();
            else if(m_get.startsWith("/music"))
                readMusicRequest();
            else if(m_get.startsWith("/picture"))
                readPictureRequest();
        }

    }

    m_socket->close();
    discardClient();

    return;
}

void Client::discardClient()
{
    m_file.close();

    m_socket->deleteLater();
    emit disconnected();

    qDebug() << "Connection closed";
}

void Client::sendPartial(qint64 offset, const QByteArray &ba)
{
    if (!m_file.isOpen()) {
        return;
    }

    QString answer = "";
    answer += "HTTP/1.1 206 Partial content \r\n";
    answer += "Server: QtMediaHub (Unix) \r\n";
    answer += "Content-Length: " + QString::number(ba.size()) +"\r\n";
    answer += "Connection: close \r\n";
    answer += "Content-Range: bytes " + QString::number(offset) + "-" + QString::number(offset+ba.size()-1) + "/" + QString::number(m_file.size()) + " \tr\n";
    answer += "Content-Type: video/avi \r\n";
    answer += "Accept-Ranges: bytes \r=n";
    answer += "\r\n";

    m_socket->write(answer.toLatin1());
    m_socket->write(ba);
    m_socket->waitForBytesWritten();
}

void Client::readVideoRequest()
{
    QString id = m_get.right(m_get.length()-m_get.lastIndexOf("/")-1);
    sendFile(getMediaUrl("video", id.toInt()).toLocalFile());

    m_socket->close();
}

void Client::readMusicRequest()
{
    QString id = m_get.right(m_get.length()-m_get.lastIndexOf("/")-1);
    sendFile(getMediaUrl("music", id.toInt()).toLocalFile());

    m_socket->close();
}

void Client::readPictureRequest()
{
}

void Client::answerOk(qint64 length)
{
    QString answer;
    answer += "HTTP/1.1 200 OK \r\n";
    answer += "Server: QtMediaHub (Unix) \r\n";
    answer += "Content-Length: " + QString::number(length) +"\r\n";
    answer += "Connection: close \r\n";
    answer += "Content-Type: application/octet-stream \r\n";
//    answer += "Accept-Ranges: bytes \r\n";
    answer += "\r\n";

    m_socket->write(answer.toLatin1());
    m_socket->waitForBytesWritten();
}

void Client::answerNotFound()
{
    QString answer;
    answer += "HTTP/1.1 404 Not Found \r\n";
    answer += "Server: QtMediaHub (Unix) \r\n";
    answer += "Connection: close \r\n";
    answer += "\r\n";

    m_socket->write(answer.toLatin1());
    m_socket->waitForBytesWritten();
}

QUrl Client::getMediaUrl(QString mediaType, int id, QString field)
{
    QSqlQuery query(Backend::instance()->mediaDatabase());
    query.setForwardOnly(true);

    QString queryString;
    queryString.append("SELECT * FROM " + mediaType);
    queryString.append(" WHERE id = " + QString::number(id) + "");
    query.prepare(queryString);

    if (!query.exec()) {
        qWarning("Error executing query: %s", qPrintable(query.lastQuery()));
        return QUrl();
    }

    // we should have just one result set
    if (!query.next()) {
        qWarning("No records found: %s", qPrintable(query.lastQuery()));
        return QUrl();
    }

    return QUrl::fromEncoded(query.record().value(field).toByteArray());
}

bool Client::sendFile(QString fileName)
{
    int chunk = 1024*16;

    m_file.setFileName(fileName);
    m_file.open(QIODevice::ReadOnly);
    if (!m_file.isOpen()) {
        qDebug() << "could not open file" << m_file.fileName();
        answerNotFound();
        return false;
    }

    answerOk(m_file.size());

    QByteArray ba;
    ba.resize(chunk);

    while (m_file.read(ba.data(), chunk) && m_socket->state() == QAbstractSocket::ConnectedState) {
        m_socket->write(ba);
        m_socket->waitForBytesWritten();
    }

    return  true;
}

