/****************************************************************************
**
** Copyright (C) 2011 Girish Ramakrishnan (girish@forwardbias.in)
**
** Use, modification and distribution is allowed without limitation,
** warranty, liability or support of any kind.
**
****************************************************************************/

#ifndef QAVAHISERVICEPUBLISHER_H
#define QAVAHISERVICEPUBLISHER_H

#include <avahi-common/thread-watch.h>
#include <avahi-common/error.h>
#include <avahi-common/malloc.h>
#include <avahi-common/alternative.h>
#include <avahi-client/client.h>
#include <avahi-client/publish.h>

#include <QObject>

class QAvahiServicePublisher : public QObject
{
    Q_OBJECT
public:
    QAvahiServicePublisher(QObject *parent = 0);
    ~QAvahiServicePublisher();

    struct Service {
        QString name;
        QString type;
        qint32 port;
        QString txtRecord;
    };

    void publish(const Service &service);
    void publish(const QString &serviceName, const QString &serviceType, qint32 port, const QString &txtRecord);

    int error() const { return m_error; }
    QString errorString() const { return m_errorString; }

    enum Notification {
        Error,
        ServicesRegistering,
        ServicesRegistered,
        ServiceNameCollision,
        ServicesUncommited,
        ServicesCommited,
        ClientConnecting,
        ServerRunning,
        ServerRegistering,
        ServerNameCollision
    };

signals:
    void changeNotification(Notification notification);

private:
    static void client_callback(AvahiClient *client, AvahiClientState state, void *userdata);
    void clientCallback(AvahiClient *client, AvahiClientState state);
    static void entry_group_callback(AvahiEntryGroup *group, AvahiEntryGroupState state, void *userdata);
    void entryGroupCallback(AvahiEntryGroup *group, AvahiEntryGroupState state);
    void initialize();
    void uninitialize();
    void doRegisterServices();

    QString m_serviceName;
    AvahiThreadedPoll *m_threaded_poll;
    AvahiClient *m_client;
    AvahiEntryGroup *m_group;
    QList<Service> m_services;
    int m_error;
    QString m_errorString;
};

#endif // QAVAHISERVICEPUBLISHER_H

