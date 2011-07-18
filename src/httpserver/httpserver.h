#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#include <QtCore>
#include <QtNetwork>

class HttpServer : public QTcpServer
{
    Q_OBJECT
    Q_PROPERTY(QString address READ address NOTIFY addressChanged)
    Q_PROPERTY(int port READ port NOTIFY portChanged)

public:
    HttpServer(quint16 port, QObject* parent = 0);

    void incomingConnection(int socket);

    QString address() const { return m_address; }
    int port() const { return serverPort(); }

signals:
    void addressChanged();
    void portChanged();

private slots:
    void getHostAddress(QHostInfo info);

private:
    QString m_base;
    QString m_address;
};

#endif // HTTPSERVER_H
