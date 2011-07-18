#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#include <QtCore>
#include <QtNetwork>

class HttpServer : public QTcpServer
{
    Q_OBJECT
    Q_PROPERTY(QString base READ base WRITE setBase NOTIFY baseChanged)

public:
    HttpServer(quint16 port, QObject* parent = 0);

    void incomingConnection(int socket);

    void setBase(const QString &b);
    QString base() const;

signals:
    void baseChanged();

private:
    QString m_base;
};

#endif // HTTPSERVER_H
