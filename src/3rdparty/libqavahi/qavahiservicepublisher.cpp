/****************************************************************************
**
** Copyright (C) 2011 Girish Ramakrishnan (girish@forwardbias.in)
**
** Use, modification and distribution is allowed without limitation,
** warranty, liability or support of any kind.
**
****************************************************************************/

#include "qavahiservicepublisher.h"
#include <QtDebug>
#include <stdarg.h>

static void QAPDEBUG(const char *fmt, ...)
{
    static const bool debug = qgetenv("DEBUG").toInt();
    if (debug) {
        va_list ap;
        va_start(ap, fmt);
        qDebug(fmt, ap);
        va_end(ap);
    }
}

// http://avahi.sourcearchive.com/documentation/0.6.25-1ubuntu2/main.html

QAvahiServicePublisher::QAvahiServicePublisher(QObject *parent)
    : QObject(parent), m_threaded_poll(0), m_client(0), m_group(0)
{
}

QAvahiServicePublisher::~QAvahiServicePublisher()
{
    uninitialize();
}

void QAvahiServicePublisher::initialize()
{
    if (m_threaded_poll)
        return;
    m_threaded_poll = avahi_threaded_poll_new();
    m_client = avahi_client_new(avahi_threaded_poll_get(m_threaded_poll), AVAHI_CLIENT_NO_FAIL, client_callback, this /* userdata */, &m_error);
    if (!m_client) {
        QAPDEBUG("Failed to create client : %s", avahi_strerror(m_error));
        avahi_threaded_poll_free(m_threaded_poll);
        m_threaded_poll = 0;
        m_errorString = avahi_strerror(m_error);
        emit changeNotification(Error);
    } else {
        avahi_threaded_poll_start(m_threaded_poll);
    }
}

void QAvahiServicePublisher::uninitialize()
{
    if (m_group) {
        avahi_entry_group_free(m_group);
    }
    if (m_threaded_poll) {
        avahi_threaded_poll_quit(m_threaded_poll);
        avahi_threaded_poll_free(m_threaded_poll);
        m_threaded_poll = 0;
    }
    if (m_client) {
        avahi_client_free(m_client);
        m_client = 0;
    }
}

void QAvahiServicePublisher::publish(const QString &name, const QString &type, qint32 port, const QString &txtRecord)
{
    Service service;
    service.name = name;
    service.type = type;
    service.port = port;
    service.txtRecord = txtRecord;

    publish(service);
}

void QAvahiServicePublisher::publish(const Service &service)
{
    initialize();

    m_services.append(service);

    if (!m_group) {
        QAPDEBUG("Creating new group");
        m_group = avahi_entry_group_new(m_client, entry_group_callback, this /* userdata */);
    }

    if (avahi_client_get_state(m_client) == AVAHI_CLIENT_S_RUNNING)
        doRegisterServices();
}

void QAvahiServicePublisher::doRegisterServices()
{
    QAPDEBUG("Adding services");
    avahi_entry_group_reset(m_group);

    for (int i = 0; i < m_services.count(); i++) {
        const Service &service = m_services[i];
        m_error = avahi_entry_group_add_service(m_group, AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC, AVAHI_PUBLISH_USE_MULTICAST, 
                                        service.name.toUtf8().constData(), service.type.toUtf8().constData(), 
                                        NULL /* domain */, NULL /* host */,
                                        service.port, service.txtRecord.toUtf8().constData(), NULL); // ## use _strlst overload
        if (m_error < 0) {
            QAPDEBUG("Failed to add service to group: %s", avahi_strerror(m_error));
            m_errorString = avahi_strerror(m_error);
            avahi_entry_group_reset(m_group);
            emit changeNotification(Error);
            return;
        }
    }

    m_error = avahi_entry_group_commit(m_group);
    if (m_error < 0) {
        QAPDEBUG("Failed to commit service group : %s", avahi_strerror(m_error));
        m_errorString = avahi_strerror(m_error);
        avahi_entry_group_reset(m_group);
        emit changeNotification(Error);
        return;
    }
    emit changeNotification(ServicesCommited);
}

void QAvahiServicePublisher::client_callback(AvahiClient *client, AvahiClientState state, void *userdata)
{
    QAvahiServicePublisher *that = reinterpret_cast<QAvahiServicePublisher *>(userdata);
    that->clientCallback(client, state);
}

void QAvahiServicePublisher::entry_group_callback(AvahiEntryGroup *group, AvahiEntryGroupState state, void *userdata)
{
    QAvahiServicePublisher *that = reinterpret_cast<QAvahiServicePublisher *>(userdata);
    that->entryGroupCallback(group, state);
}

void QAvahiServicePublisher::clientCallback(AvahiClient *client, AvahiClientState state)
{
    switch (state) {
    case AVAHI_CLIENT_FAILURE:
        QAPDEBUG("Connection failure: %s", avahi_strerror(avahi_client_errno(client)));
        m_error = AVAHI_CLIENT_FAILURE;
        m_errorString = avahi_strerror(avahi_client_errno(client));
        emit changeNotification(Error);
        uninitialize();
        break;
    case AVAHI_CLIENT_CONNECTING:
        QAPDEBUG("Client is connecting");
        emit changeNotification(ClientConnecting);
        break;
    case AVAHI_CLIENT_S_RUNNING:
        QAPDEBUG("Server running");
        emit changeNotification(ServerRunning);
        if (!m_services.isEmpty())
            doRegisterServices();
        break;
    case AVAHI_CLIENT_S_COLLISION:
        QAPDEBUG("Server name Collission");
        emit changeNotification(ServerNameCollision);
        if (m_group)
            avahi_entry_group_reset(m_group);
        break;
    case AVAHI_CLIENT_S_REGISTERING:
        QAPDEBUG("Registering");
        emit changeNotification(ServerRegistering);
        if (m_group)
            avahi_entry_group_reset(m_group);
        break;
    }
}

void QAvahiServicePublisher::entryGroupCallback(AvahiEntryGroup *group, AvahiEntryGroupState state)
{
    switch (state) {
    case AVAHI_ENTRY_GROUP_ESTABLISHED:
        QAPDEBUG("All fine and dandy");
        emit changeNotification(ServicesRegistered);
        return;
    case AVAHI_ENTRY_GROUP_COLLISION: {
        QAPDEBUG("Oops, service name Collission");
        // There's no way to tell which name caused a collision. Rename all!
        for (int i = 0; i < m_services.count(); i++) {
            QString name = m_services[i].name;
            char *newName = avahi_alternative_service_name(name.toUtf8().constData());
            QAPDEBUG("Service %s renamed to %s", qPrintable(name), newName);
            m_services[i].name = QString::fromUtf8(newName);
            avahi_free(newName);
        }
        emit changeNotification(ServiceNameCollision);
        doRegisterServices();
        return;
                                      }
    case AVAHI_ENTRY_GROUP_FAILURE:
        QAPDEBUG("Entry group failure : %s", avahi_strerror(avahi_client_errno(avahi_entry_group_get_client(group))));
        m_error = avahi_client_errno(avahi_entry_group_get_client(group));
        m_errorString = avahi_strerror(m_error);
        emit changeNotification(Error);
        uninitialize();
        return;
    case AVAHI_ENTRY_GROUP_REGISTERING:
        QAPDEBUG("Registring the group");
        emit changeNotification(ServicesRegistering);
        return;
    case AVAHI_ENTRY_GROUP_UNCOMMITED:
        QAPDEBUG("Entry group uncommitted");
        emit changeNotification(ServicesUncommited);
        return;
    default:
        QAPDEBUG("Unhandled state in entryGroupCallback: %d", state);
        return;
    }
}

