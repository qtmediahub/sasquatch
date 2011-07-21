/****************************************************************************

This file is part of the QtMediaHub project on http://www.gitorious.org.

Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).*
All rights reserved.

Contact:  Nokia Corporation (qt-info@nokia.com)**

You may use this file under the terms of the BSD license as follows:

"Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
* Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
* Neither the name of Nokia Corporation and its Subsidiary(-ies) nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."

****************************************************************************/

#include "httpclient.h"

#include <QtSql>

#include "backend.h"
#include "httpserver.h"

HttpClient::HttpClient(int sockfd, HttpServer *server, QObject *parent) :
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

void HttpClient::readClient()
{
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
        qDebug() << m_request;

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

void HttpClient::discardClient()
{
    m_file.close();

    m_socket->deleteLater();
    emit disconnected();

    qDebug() << "Connection closed";
}


void HttpClient::readVideoRequest()
{
    QString id = m_get.right(m_get.length()-m_get.lastIndexOf("/")-1);

    if (m_request.contains("Range")) {
        QString offsetString = m_request.value("Range");
        offsetString.remove(0, 6);
        qint64 offset = offsetString.split("-").at(0).toLongLong();
        sendPartial(getMediaUrl("video", id.toInt()).toLocalFile(), offset);
    } else {
        sendFile(getMediaUrl("video", id.toInt()).toLocalFile());
    }

    m_socket->close();
}

void HttpClient::readMusicRequest()
{
    QString id = m_get.right(m_get.length()-m_get.lastIndexOf("/")-1);

    if (m_request.contains("Range")) {
        QString offsetString = m_request.value("Range");
        offsetString.remove(0, 6);
        qint64 offset = offsetString.split("-").at(0).toLongLong();
        sendPartial(getMediaUrl("music", id.toInt()).toLocalFile(), offset);
    } else {
        sendFile(getMediaUrl("music", id.toInt()).toLocalFile());
    }


    m_socket->close();
}

void HttpClient::readPictureRequest()
{
}

void HttpClient::answerOk(qint64 length)
{
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

void HttpClient::answerNotFound()
{
    QByteArray answer;
    answer += "HTTP/1.1 404 Not Found \r\n";
    answer += "Server: QtMediaHub (Unix) \r\n";
    answer += "Connection: close \r\n";
    answer += "\r\n";

    m_socket->write(answer);
    m_socket->waitForBytesWritten();
}

QUrl HttpClient::getMediaUrl(QString mediaType, int id, QString field)
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

bool HttpClient::sendFile(QString fileName)
{
    int chunk = 1024*16;

    qDebug() << "start send file";

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

    if (m_socket->state() != QAbstractSocket::ConnectedState)
        qDebug() << "fully sent connection closed";

    return  true;
}

bool HttpClient::sendPartial(QString fileName, qint64 offset)
{
    int chunk = 1024*16;

    m_file.setFileName(fileName);
    m_file.open(QIODevice::ReadOnly);

    if (!m_file.isOpen()) {
        qDebug() << "could not open file" << m_file.fileName();
        answerNotFound();
        return false;
    }

    qDebug() << "sendPartial offset" << offset;

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

    qDebug() << "sendPartial sent" << (m_socket->state() == QAbstractSocket::ConnectedState);

    return  true;
}
