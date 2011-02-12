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
        char buf[1024];
        va_list ap;
        va_start(ap, fmt);
        vsnprintf(buf, sizeof(buf), fmt, ap);
        va_end(ap);
        qDebug("QAvahiServiceBrowserModel: %s", buf);
    }
}

QAvahiServiceBrowserModel::QAvahiServiceBrowserModel(QObject *parent)
    : QAbstractListModel(parent), m_client(0), m_options(QAvahiServiceBrowserModel::NoOptions), m_browserType(NoBrowserType),
      m_browser(0), m_autoResolve(true)
{
    qRegisterMetaType<QAvahiServiceBrowserModel::Service>();

    QHash<int, QByteArray> hash = QAbstractListModel::roleNames();
    hash[NameRole] = "name";
    hash[TypeRole] = "type";
    hash[DomainRole] = "domain";
    hash[ProtocolRole] = "protocol";
    hash[InterfaceRole] = "interface";
    hash[HostNameRole] = "hostname";
    hash[AddressRole] = "address";
    hash[PortRole] = "port";
    hash[TxtRole] = "txt";
    setRoleNames(hash);

    initialize();
}

QAvahiServiceBrowserModel::~QAvahiServiceBrowserModel()
{
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

void QAvahiServiceBrowserModel::uninitialize()
{
    if (m_browser) {
        if (m_browserType == ServiceBrowser) {
            avahi_service_browser_free(static_cast<AvahiServiceBrowser *>(m_browser));
        }
        m_browserType = NoBrowserType;
        m_browser = 0;
    }
    if (m_client) {
        avahi_client_free(m_client);
        m_client = 0;
    }
}

void QAvahiServiceBrowserModel::resetModel()
{
    beginResetModel();
    m_services.clear();
    m_rowToServiceIndex.clear();
    endResetModel();
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
        // reappears. Recreate the client.
        uninitialize();
        resetModel();
        initialize();
        break;
    case AVAHI_CLIENT_CONNECTING:
        QABDEBUG("Client is connecting");
        emit changeNotification(ClientConnecting);
        break;
    case AVAHI_CLIENT_S_RUNNING:
        QABDEBUG("Server running");
        emit changeNotification(ServerRunning);
        if (!m_serviceType.isEmpty())
            doBrowse(client);
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

static bool shouldAdd(QAvahiServiceBrowserModel::Options options, const QAvahiServiceBrowserModel::Service &service)
{
    if ((options & QAvahiServiceBrowserModel::HideLocal) && service.isLocal())
        return false;

    if ((options & QAvahiServiceBrowserModel::HideSameLocalClient) && service.isSameLocalClient())
        return false;

    return true;
}
void QAvahiServiceBrowserModel::browserCallback(AvahiServiceBrowser *browser, AvahiIfIndex interface, AvahiProtocol protocol, AvahiBrowserEvent event, const char *name, const char *type, const char *domain, AvahiLookupResultFlags flags)
{
    AvahiClient *client = avahi_service_browser_get_client(browser);

    Service service;
    service.name = name;
    service.type = type;
    service.domain = domain;
    service.protocol = protocol;
    service.interface = interface;
    service.flags = flags;

    switch (event) {
    case AVAHI_BROWSER_FAILURE:
        QABDEBUG("Browser failure : %s", avahi_strerror(avahi_client_errno(client)));
        m_error = avahi_client_errno(client);
        m_errorString = avahi_strerror(m_error);
        emit changeNotification(Error);
        uninitialize();
        resetModel();
        initialize();
        break;

    case AVAHI_BROWSER_NEW: {
        QABDEBUG("New service name:%s type:%s domain:%s flags:0x%x", name, type, domain, flags);
        if (shouldAdd(m_options, service)) { // ## test if flags is reliable at this point
            beginInsertRows(QModelIndex(), m_rowToServiceIndex.count(), m_rowToServiceIndex.count());
            m_services.append(service);
            m_rowToServiceIndex.append(m_services.count()-1);
            endInsertRows();
        } else {
            m_services.append(service);
        }
        if (m_autoResolve)
            resolve(&m_services.last());
                            }
        break;

    case AVAHI_BROWSER_REMOVE: {
        QABDEBUG("Remove service");
        const int idx = serviceIndex(name, type, domain, interface, protocol);
        const int row = m_rowToServiceIndex.indexOf(idx);
        if (idx == -1) {
            qWarning() << "Removing non-existent service";
            return;
        }
        if (row == -1) {
            m_services.removeAt(idx);
        } else {
            beginRemoveRows(QModelIndex(), row, row);
            m_services.removeAt(idx);
            QList<int>::iterator it = m_rowToServiceIndex.erase(m_rowToServiceIndex.begin() + row);
            for (; it != m_rowToServiceIndex.end(); ++it)
                --(*it);
            endRemoveRows();
        }
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

void QAvahiServiceBrowserModel::browse(const QString &serviceType, QAvahiServiceBrowserModel::Options options)
{
    m_serviceType = serviceType;
    m_options = options;

    resetModel();

    if (m_browser) { // already browsing; remove existing browser
        if (m_browserType == ServiceBrowser)
            avahi_service_browser_free(static_cast<AvahiServiceBrowser *>(m_browser));
        m_browserType = NoBrowserType;
        m_browser = 0;
    }

    if (avahi_client_get_state(m_client) != AVAHI_CLIENT_S_RUNNING) { // server not running, we should try later
        QABDEBUG("Server is not running yet, will browse later. Check if avahi-daemon is running");
        return;
    }

    doBrowse(m_client);
}

static int toAvahiProtocol(QAvahiServiceBrowserModel::Options options)
{
    if (options & QAvahiServiceBrowserModel::HideIPv4)
        return AVAHI_PROTO_INET6;
    else if (options & QAvahiServiceBrowserModel::HideIPv6)
        return AVAHI_PROTO_INET;
    else
        return AVAHI_PROTO_UNSPEC;
}

// This function takes client as argument because it's called from clientCallback. Since
// clientCallback maybe called from avahi_client_new, m_client may still be 0.
void QAvahiServiceBrowserModel::doBrowse(AvahiClient *client)
{
    QABDEBUG("Creating browser");
    int proto = toAvahiProtocol(m_options);

    m_browser = avahi_service_browser_new(client, AVAHI_IF_UNSPEC, proto, m_serviceType.toUtf8().constData(),
                                          NULL /* domain */, (AvahiLookupFlags)0, /*AVAHI_LOOKUP_USE_MULTICAST*/
                                          browser_callback, this /* userdata */);

    if (!m_browser) {
        QABDEBUG("Failed to create browser");
    } else {
        m_browserType = ServiceBrowser;
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
    int row = m_rowToServiceIndex.indexOf(idx);

    if (event == AVAHI_RESOLVER_FAILURE) {
        QABDEBUG("Failed to resolve");
        service.resolved = false;
        emit resolved(idx);
    } else if (event == AVAHI_RESOLVER_FOUND) {
        service.hostName = host_name;
        if (address->proto == AVAHI_PROTO_INET) {
            service.address = QHostAddress(ntohl(address->data.ipv4.address));
        } else if (address->proto == AVAHI_PROTO_INET6) {
            service.address = QHostAddress((quint8 *)address->data.ipv6.address);
        }
        service.port = port;
        service.textRecords.clear();
        for (AvahiStringList *l = txt; l != NULL; l = avahi_string_list_get_next(l)) {
            service.textRecords.append((const char *)l->text);
        }
        service.flags = flags;
        service.resolved = true;
        QABDEBUG("Service found : name:%s type:%s domain:%s port:%d address:%s flags:0x%x", name, type, domain, port, qPrintable(service.address.toString()), service.flags);
        QABDEBUG("TXT: %s", qPrintable(service.textRecords.join(",")));
        if (row != -1) {
            emit resolved(row);
            emit dataChanged(createIndex(row, 0), createIndex(row, 8));
        } else if (shouldAdd(m_options, service)) {
            QList<int>::iterator it = qLowerBound(m_rowToServiceIndex.begin(), m_rowToServiceIndex.end(), idx);
            const int loc = it - m_rowToServiceIndex.begin();
            beginInsertRows(QModelIndex(), loc, loc);
            m_rowToServiceIndex.insert(it, idx);
            endInsertRows();
        }
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
    Service &service = m_services[m_rowToServiceIndex.value(idx)];
    resolve(&service);

    if (!service.resolver) {
        m_error = avahi_client_errno(m_client);
        m_errorString = avahi_strerror(m_error);
        QABDEBUG("Cannot create resolver. %s\n", qPrintable(m_errorString));
        emit resolved(idx);
    }
}

void QAvahiServiceBrowserModel::resolve(Service *service)
{
    if (service->resolver) // already resolving
        return;

    if (avahi_client_get_state(m_client) != AVAHI_CLIENT_S_RUNNING) // server not running
        QABDEBUG("Server is not running yet, cannot resolve. Check if avahi-daemon is running");

    service->resolver = avahi_service_resolver_new(m_client, service->interface, service->protocol, service->name.toUtf8().constData(), 
                                                  service->type.toUtf8().constData(), service->domain.toUtf8().constData(),
                                                  AVAHI_PROTO_UNSPEC, (AvahiLookupFlags)0, resolver_callback, this /*userdata*/);
}

int QAvahiServiceBrowserModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_rowToServiceIndex.count();
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
    const int row = m_rowToServiceIndex.value(index.row());
    const QAvahiServiceBrowserModel::Service &service = m_services.at(row);
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
    case NameRole: return service.name;
    case TypeRole: return service.type;
    case DomainRole: return service.domain;
    case ProtocolRole: return service.protocol;
    case InterfaceRole: return service.interface;
    case HostNameRole: return service.hostName;
    case AddressRole: return service.address.toString();
    case PortRole: return service.port;
    case TxtRole: return service.textRecords.join(",");
    default:
        break;
    }
    return QVariant();
}

