/****************************************************************************
**
** Copyright (C) 2011 Girish Ramakrishnan (girish@forwardbias.in)
**
** Use, modification and distribution is allowed without limitation,
** warranty, liability or support of any kind.
**
****************************************************************************/

#include "qavahiservicebrowsermodel.h"
#include <QtDebug>
#include <QtGui>
#include <arpa/inet.h>

static void QABDEBUG(const char *fmt, ...)
{
    static const bool debug = qgetenv("DEBUG").toInt();
    if (debug) {
        va_list ap;
        va_start(ap, fmt);
        qDebug(fmt, ap);
        va_end(ap);
    }
}

QAvahiServiceBrowserModel::QAvahiServiceBrowserModel(QObject *parent)
    : QAbstractListModel(parent), m_client(0), m_protocol(QAbstractSocket::UnknownNetworkLayerProtocol), m_browser(0), m_autoResolve(true),
      m_browseWhenServerRunning(false)
{
    qRegisterMetaType<QAvahiServiceBrowserModel::Service>();
    initialize();
}

QAvahiServiceBrowserModel::~QAvahiServiceBrowserModel()
{
}

void QAvahiServiceBrowserModel::client_callback(AvahiClient *client, AvahiClientState state, void *userdata)
{
    QAvahiServiceBrowserModel *browser = reinterpret_cast<QAvahiServiceBrowserModel *>(userdata);
    browser->clientCallback(client, state);
}

void QAvahiServiceBrowserModel::clientCallback(AvahiClient *client, AvahiClientState state)
{
    switch (state) {
    case AVAHI_CLIENT_FAILURE:
        QABDEBUG("Connection failure : %s", avahi_strerror(avahi_client_errno(client)));
        m_error = AVAHI_CLIENT_FAILURE;
        m_errorString = avahi_strerror(avahi_client_errno(client));
        emit changeNotification(Error);
        // Once a client loses connection to the daemon, it doesn't seem to reconnect when the daemon
        // reappears. Recreate the client. Note the model still contains the entries.
        uninitialize();
        initialize();
        break;
    case AVAHI_CLIENT_CONNECTING:
        QABDEBUG("Client is connecting");
        emit changeNotification(ClientConnecting);
        break;
    case AVAHI_CLIENT_S_RUNNING:
        QABDEBUG("Server running");
        emit changeNotification(ServerRunning);
        if (m_browseWhenServerRunning) {
            doBrowse();
            m_browseWhenServerRunning = false;
        }
        break;
    case AVAHI_CLIENT_S_COLLISION:
        QABDEBUG("Server name Collission");
        emit changeNotification(ServerNameCollision);
        break;
    case AVAHI_CLIENT_S_REGISTERING:
        QABDEBUG("Registering");
        emit changeNotification(ServerRegistering);
        break;
    default:
        QABDEBUG("Unhandled state in clientCallback: %d", state);
        break;
    }
}

void QAvahiServiceBrowserModel::browser_callback(AvahiServiceBrowser *browser, AvahiIfIndex interface, AvahiProtocol protocol, AvahiBrowserEvent event, const char *name, const char *type, const char *domain, AvahiLookupResultFlags flags, void *userdata)
{
    QAvahiServiceBrowserModel *that = reinterpret_cast<QAvahiServiceBrowserModel *>(userdata);
    that->browserCallback(browser, interface, protocol, event, name, type, domain, flags);
}

int QAvahiServiceBrowserModel::serviceIndex(const char *name, const char *type, const char *domain, AvahiIfIndex interface, AvahiProtocol protocol)
{
    for (int i = 0; i < m_services.count(); i++) {
        if (m_services[i].name == name && m_services[i].type == type &&  m_services[i].domain == domain
            && m_services[i].interface == interface && m_services[i].protocol == protocol)
            return i;
    }
    return -1;
}

void QAvahiServiceBrowserModel::browserCallback(AvahiServiceBrowser *browser, AvahiIfIndex interface, AvahiProtocol protocol, AvahiBrowserEvent event, const char *name, const char *type, const char *domain, AvahiLookupResultFlags flags)
{
    Q_UNUSED(flags);
    AvahiClient *client = avahi_service_browser_get_client(browser);

    Service service;
    service.name = name;
    service.type = type;
    service.domain = domain;
    service.protocol = protocol;
    service.interface = interface;

    switch (event) {
    case AVAHI_BROWSER_FAILURE:
        QABDEBUG("Browser failure : %s", avahi_strerror(avahi_client_errno(client)));
        m_error = avahi_client_errno(client);
        m_errorString = avahi_strerror(m_error);
        emit changeNotification(Error);
        uninitialize();
        initialize();
        break;

    case AVAHI_BROWSER_NEW: {
        QABDEBUG("New service name:%s type:%s domain:%s", name, type, domain);
        beginInsertRows(QModelIndex(), m_services.count(), m_services.count()+1);
        m_services.append(service);
        endInsertRows();
        if (m_autoResolve)
            resolve(m_services.count()-1);
                            }
        break;

    case AVAHI_BROWSER_REMOVE: {
        QABDEBUG("Remove service");
        int idx = serviceIndex(name, type, domain, interface, protocol);
        if (idx == -1) {
            qWarning() << "Removing non-existent service";
            return;
        }
        beginRemoveRows(QModelIndex(), idx, idx);
        m_services.removeAt(idx);
        endRemoveRows();
        break;
                               }

    case AVAHI_BROWSER_ALL_FOR_NOW:
        QABDEBUG("All for now");
        emit done();
        break;

    case AVAHI_BROWSER_CACHE_EXHAUSTED:
        QABDEBUG("Cache exhausted");
        break;

    default:
        break;
    }

}

void QAvahiServiceBrowserModel::initialize()
{
    const AvahiPoll *poll_api = avahi_qt_poll_get();

    m_client = avahi_client_new(poll_api, AVAHI_CLIENT_NO_FAIL,
                                client_callback, this /* userdata */, &m_error);
    if (!m_client) {
        QABDEBUG("Failed to create client %s", avahi_strerror(m_error));
        m_errorString = avahi_strerror(m_error);
        emit changeNotification(Error);
    }
}

void QAvahiServiceBrowserModel::browse(const QString &serviceType, QAbstractSocket::NetworkLayerProtocol protocol)
{
    m_serviceType = serviceType;
    m_protocol = protocol;

    beginResetModel();
    m_services.clear();
    endResetModel();

    if (m_browser) { // already browsing
        avahi_service_browser_free(m_browser);
        m_browser = 0;
    }

    if (avahi_client_get_state(m_client) != AVAHI_CLIENT_S_RUNNING) { // server not running, we should try later
        QABDEBUG("Server is not running yet, will browse later. Check if avahi-daemon is running");
        m_browseWhenServerRunning = true;
        return;
    }

    doBrowse();
}

void QAvahiServiceBrowserModel::doBrowse()
{
    int proto;
    switch (m_protocol) {
    case QAbstractSocket::IPv4Protocol: proto = AVAHI_PROTO_INET; break;
    case QAbstractSocket::IPv6Protocol: proto = AVAHI_PROTO_INET6; break;
    case QAbstractSocket::UnknownNetworkLayerProtocol: 
    default: proto = AVAHI_PROTO_UNSPEC; break;
    }

    m_browser = avahi_service_browser_new(m_client, AVAHI_IF_UNSPEC, proto, m_serviceType.toUtf8().constData(),
                                          NULL /* domain */, (AvahiLookupFlags)0, /*AVAHI_LOOKUP_USE_MULTICAST*/
                                          browser_callback, this /* userdata */);
}

void QAvahiServiceBrowserModel::uninitialize()
{
    if (m_browser) {
        avahi_service_browser_free(m_browser);
        m_browser = 0;
    }
    if (m_client) {
        avahi_client_free(m_client);
        m_client = 0;
    }
}

void QAvahiServiceBrowserModel::resolver_callback(AvahiServiceResolver *r, AvahiIfIndex interface, AvahiProtocol protocol, AvahiResolverEvent event,
    const char *name, const char *type, const char *domain, const char *host_name, const AvahiAddress *address,
    uint16_t port, AvahiStringList *txt, AvahiLookupResultFlags flags, void* userdata)
{
    QAvahiServiceBrowserModel *browser = reinterpret_cast<QAvahiServiceBrowserModel *>(userdata);
    browser->resolverCallback(r, interface, protocol, event, name, type, domain, host_name, address, port, txt, flags);
}

void QAvahiServiceBrowserModel::resolverCallback(AvahiServiceResolver *r, AvahiIfIndex interface, AvahiProtocol protocol, AvahiResolverEvent event,
    const char *name, const char *type, const char *domain, const char *host_name, const AvahiAddress *address,
    uint16_t port, AvahiStringList *txt, AvahiLookupResultFlags flags)
{
    int idx = serviceIndex(name, type, domain, interface, protocol);
    Q_ASSERT(idx != -1);
    Service &service = m_services[idx];

    if (event == AVAHI_RESOLVER_FAILURE) {
        QABDEBUG("Failed to resolve");
        emit resolved(idx);
    } else if (event == AVAHI_RESOLVER_FOUND) {
        service.hostName = host_name;
        if (address->proto == AVAHI_PROTO_INET) {
            service.address = QHostAddress(ntohl(address->data.ipv4.address));
        } else if (address->proto == AVAHI_PROTO_INET6) {
            service.address = QHostAddress((quint8 *)address->data.ipv6.address);
        }
        service.port = port;
        for (AvahiStringList *l = txt; l != NULL; l = avahi_string_list_get_next(l)) {
            service.textRecords.append((const char *)l->text);
        }
        service.flags = flags;

        QABDEBUG("Service found : name:%s type:%s domain:%s port:%d address:%s", name, type, domain, port, qPrintable(service.address.toString()));
        QABDEBUG("TXT: %s", qPrintable(service.textRecords.join(",")));
        emit resolved(idx);
        emit dataChanged(createIndex(idx, 0), createIndex(idx, 8));
    } else {
        QABDEBUG("Unhandled event in resolverCallback : %d", event);
    }

    avahi_service_resolver_free(r);
    service.resolver = 0;
}

void QAvahiServiceBrowserModel::resolve(const QModelIndex &index)
{
    resolve(index.row());
}

void QAvahiServiceBrowserModel::resolve(int idx)
{
    Service &service = m_services[idx];
    if (service.resolver) // already resolving
        return;

    if (avahi_client_get_state(m_client) != AVAHI_CLIENT_S_RUNNING) // server not running
        QABDEBUG("Server is not running yet, cannot resolve. Check if avahi-daemon is running");

    service.resolver = avahi_service_resolver_new(m_client, service.interface, service.protocol, service.name.toUtf8().constData(), 
                                                  service.type.toUtf8().constData(), service.domain.toUtf8().constData(),
                                                  AVAHI_PROTO_UNSPEC, (AvahiLookupFlags)0, resolver_callback, this /*userdata*/);

    if (!service.resolver) {
        m_error = avahi_client_errno(m_client);
        m_errorString = avahi_strerror(m_error);
        QABDEBUG("Cannot create resolver. %s\n", qPrintable(m_errorString));
        emit resolved(idx);
    }
}

int QAvahiServiceBrowserModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_services.count();
}

int QAvahiServiceBrowserModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return 9;
}

QVariant QAvahiServiceBrowserModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Vertical || role != Qt::DisplayRole)
        return QVariant();
    switch (section) {
    case 0: return tr("Name");
    case 1: return tr("Type");
    case 2: return tr("Domain");
    case 3: return tr("Protocol");
    case 4: return tr("Interface");
    case 5: return tr("Host name");
    case 6: return tr("Address");
    case 7: return tr("Port");
    case 8: return tr("TXT");
    default: return QVariant();
    }
}

QVariant QAvahiServiceBrowserModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();
    const QAvahiServiceBrowserModel::Service &service = m_services.at(index.row());
    const int col = index.column();
    switch (role) {
    case Qt::DisplayRole: 
        if (col == 0) return service.name;
        if (col == 1) return service.type;
        if (col == 2) return service.domain;
        if (col == 3) return service.protocol;
        if (col == 4) return service.interface;
        if (col == 5) return service.hostName;
        if (col == 6) return service.address.toString();
        if (col == 7) return service.port;
        if (col == 8) return service.textRecords.join(",");
        break;
    default: break;
    }
    return QVariant();
}

