/****************************************************************************

This file is part of the QtMediaHub project on http://www.qtmediahub.com

Copyright (c) 2011 Nokia Corporation and/or its subsidiary(-ies).*
All rights reserved.

Contact:  Girish Ramakrishnan girish@forwardbias.in
Contact:  Donald Carr sirspudd@gmail.com
Contact:  Nokia Corporation johannes.zellner@nokia.com

You may use this file under the terms of the BSD license as follows:

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
* Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
* Neither the name of Nokia Corporation and its Subsidiary(-ies) nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

****************************************************************************/

#include <unistd.h>

#include "httpclient.h"
#include "httpclient_p.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlError>

#include "httpserver.h"
#include "global.h"
#include "skinmanager.h"


#ifdef QT5
HttpClient::HttpClient(qintptr sockfd, HttpServer *server, SkinManager *skinManager, QObject *parent)
#else
HttpClient::HttpClient(int sockfd, HttpServer *server, SkinManager *skinManager, QObject *parent)
#endif
    : QThread(parent)
    , m_server(server)
    , m_skinManager(skinManager)
    , m_sockfd(sockfd)
{
}
void HttpClient::run()
{
    d.reset(new HttpClientPrivate(m_sockfd, m_server, m_skinManager, 0));  // No parent because HttpClientPrivate lifes in another thread then HttpClient

    connect(d.data(), SIGNAL(disconnected()), this, SLOT(quit()));
    connect(this, SIGNAL(finished()), this, SLOT(deleteLater()));

    exec();
}


#ifdef QT5
HttpClientPrivate::HttpClientPrivate(qintptr sockfd, HttpServer *server, SkinManager* skinManager, QObject *parent) :
#else
HttpClientPrivate::HttpClientPrivate(int     sockfd, HttpServer *server, SkinManager* skinManager, QObject *parent) :
#endif
    QObject(parent),
    m_sockfd(sockfd),
    m_server(server),
    m_skinManager(skinManager)

{
    Q_ASSERT(this->thread() == QThread::currentThread());

    m_socket = new QTcpSocket(this);
    if (!m_socket->setSocketDescriptor(m_sockfd)) {
        emit error(m_socket->error());
        return;
    }

    connect(m_socket, SIGNAL(readyRead()), this, SLOT(readClient()));
    connect(m_socket, SIGNAL(disconnected()), this, SIGNAL(disconnected()));
}

void HttpClientPrivate::readClient()
{
    Q_ASSERT(this->thread() == QThread::currentThread());

    if (m_socket->canReadLine()) {
        QByteArray bytesRead = m_socket->readLine();
        QStringList tokens = QString(bytesRead).split(QRegExp("[ \r\n][ \r\n]*"));

        while(m_socket->canReadLine()) {
            bytesRead = m_socket->readLine();
            QList<QByteArray> lineTokens = bytesRead.split(':');
            if (lineTokens.size() == 2) {
                m_request.insert(lineTokens[0], lineTokens[1].trimmed().replace("\r\n",""));

            }
        }

        if (tokens[0] == "GET") {
            QString get = tokens[1];

            if(get.startsWith("/qml"))
                readQmlRequest(get);
            else
                readMediaRequest(get);
        }
    }
}

void HttpClientPrivate::readQmlRequest(const QString& get)
{
    QStringList requestTokens = get.split('/', QString::SkipEmptyParts);
    if(requestTokens.length() != 3) {
        printRequestFormatErrorMessage(get);
        return;
    }

    QString skinName = requestTokens.at(1);
    QString fileName = requestTokens.at(2);

    Skin* skin = (m_skinManager->skins().value(skinName, 0));
    if(skin == 0) {
        qWarning() << "HttpClient: requested skin: " << skinName << " .. but this skin is unknown";
        return;
    }

    sendFile( skin->path() + "/remoteqml/" + fileName);
}

void HttpClientPrivate::readMediaRequest(const QString& get)
{
    QStringList requestTokens = get.split('/', QString::SkipEmptyParts);
    QString id;
    bool thumbnail;

    QString mediaType = requestTokens.at(0);
    if(     mediaType.compare("music", Qt::CaseInsensitive)   &&
            mediaType.compare("picture", Qt::CaseInsensitive) &&
            mediaType.compare("video", Qt::CaseInsensitive)   ) {
        printRequestFormatErrorMessage(get);
        return;
    }

    if(requestTokens.length() == 3) {  // it must be a thumbnail request!
        if(requestTokens.at(1).compare("thumbnail", Qt::CaseInsensitive)) {
            printRequestFormatErrorMessage(get);
            return;
        }

        if(!mediaType.compare("music", Qt::CaseInsensitive)) {
            qWarning() << "HttpClient: thumbnail for music requested, but thumbnails for music are currently not supported";
            return;
        }

        thumbnail = true;
        id = requestTokens.at(2);
    }
    else {  // it's a normal (not thumbnail) request!
        if(requestTokens.length() == 2) {
            thumbnail = false;
            id = requestTokens.at(1);
        }
    }

    bool ok;
    int intId = id.toInt(&ok);
    if(!ok) {
        printRequestFormatErrorMessage(get);
        return;
    }

    if (m_request.contains("Range")) {
        if(!mediaType.compare("picture", Qt::CaseInsensitive)) {
            qWarning() << "HttpClient: picture-request doesn't support Range parameters";
            return;
        }

        QString offsetString = m_request.value("Range");
        offsetString.remove(0, 6);
        qint64 offset = offsetString.split("-").at(0).toLongLong();
        sendPartial(getMediaUrl(mediaType, intId).toLocalFile(), offset);
    }
    else {
        sendFile(getMediaUrl(mediaType, intId, thumbnail ? "thumbnail" : "filepath").toLocalFile());
    }
}

void HttpClientPrivate::printRequestFormatErrorMessage(const QString &get)
{
    qWarning() << "HttpClient: the http-request was of wrong format: " <<
                  get << "\n   ... the right format is: /<type>/[thumbnail/]<id> or /<type>/<id> for media and /qml/<skin>/<qml-file> for qml files" <<
                  "   (supported types: picture, music, video); <id> must be convertable to int";
}

void HttpClientPrivate::answerOk(qint64 length)
{
    Q_ASSERT(this->thread() == QThread::currentThread());

    QByteArray answer;
    answer += "HTTP/1.1 200 OK \r\n";
    answer += "Server: QtMediaHub (Unix) \r\n";
    answer += "Content-Length: " + QString::number(length) +"\r\n";
    answer += "Connection: close \r\n";
    answer += "Content-Type: application/octet-stream \r\n";
    answer += "Accept-Ranges: bytes \r\n";
    answer += "\r\n";

    m_socket->write(answer);
    m_socket->waitForBytesWritten();
}

void HttpClientPrivate::answerNotFound()
{
    Q_ASSERT(this->thread() == QThread::currentThread());

    QByteArray answer;
    answer += "HTTP/1.1 404 Not Found \r\n";
    answer += "Server: QtMediaHub (Unix) \r\n";
    answer += "Connection: close \r\n";
    answer += "\r\n";

    m_socket->write(answer);
    m_socket->waitForBytesWritten();
}

QUrl HttpClientPrivate::getMediaUrl(QString mediaType, int id, QString field)
{
    Q_ASSERT(this->thread() == QThread::currentThread());

    QSqlQuery query(QSqlDatabase::database(DEFAULT_DATABASE_CONNECTION_NAME));
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

    return QUrl(query.record().value(field).toByteArray());
}

bool HttpClientPrivate::sendFile(QString fileName)
{
    Q_ASSERT(this->thread() == QThread::currentThread());

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
        sleep(0.1);
    }

    m_file.close();
    return true;
}

bool HttpClientPrivate::sendPartial(QString fileName, qint64 offset)
{
    Q_ASSERT(this->thread() == QThread::currentThread());

    int chunk = 1024*16;

    m_file.setFileName(fileName);
    m_file.open(QIODevice::ReadOnly);

    if (!m_file.isOpen()) {
        qDebug() << "could not open file" << m_file.fileName();
        answerNotFound();
        return false;
    }

    QByteArray ba;
    ba.resize(chunk);
    if (!m_file.seek(offset)) {
        qDebug() << "could not seek to offset";
        return false;
    }

    QByteArray answer = "";
    answer += "HTTP/1.1 206 Partial content \r\n";
    answer += "Server: QtMediaHub (Unix) \r\n";
    answer += "Content-Length: " + QString::number(m_file.size()-offset) +"\r\n";
    answer += "Connection: close \r\n";
    answer += "Content-Range: bytes " + QString::number(offset) + "-" + QString::number(m_file.size()-1) + "/" + QString::number(m_file.size()) + " \t\n";
    answer += "Content-Type: application/octet-stream \r\n";
    answer += "Accept-Ranges: bytes \r\n";
    answer += "\r\n";

    m_socket->write(answer);

    while (m_file.read(ba.data(), chunk) && m_socket->state() == QAbstractSocket::ConnectedState) {
        m_socket->write(ba);
        m_socket->waitForBytesWritten();
        sleep(0.1);
    }

    return  true;
}
