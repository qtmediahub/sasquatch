#ifndef CLIENT_H
#define CLIENT_H

#include <QObject>
#include <QtNetwork>

class HttpServer;

class Client : public QObject
{
    Q_OBJECT
public:
    explicit Client(int sockfd, HttpServer *server, QObject *parent = 0);

signals:
    void error(QTcpSocket::SocketError socketError);
    void disconnected();

private slots:
    void readClient();
    void discardClient();

private:
    void sendPartial(qint64 offset, const QByteArray &ba);
    void readVideoRequest();
    void readMusicRequest();
    void readPictureRequest();

    QUrl getMediaUrl(QString mediaType, int id, QString field = "uri");
    bool sendFile(QString fileName);

    void answerOk(qint64 length);
    void answerNotFound();

    QTcpSocket *m_socket;
    HttpServer *m_server;
    QFile m_file;
    QHash<QString, QString> m_request;
    QString m_get;
};

#endif // CLIENT_H
