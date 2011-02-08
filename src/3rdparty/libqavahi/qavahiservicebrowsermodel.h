/****************************************************************************
**
** Copyright (C) 2011 Girish Ramakrishnan (girish@forwardbias.in)
**
** Use, modification and distribution is allowed without limitation,
** warranty, liability or support of any kind.
**
****************************************************************************/

#ifndef QAVAHISERVICEBROWSERMODEL_H
#define QAVAHISERVICEBROWSERMODEL_H

#include <avahi-client/client.h>
#include <avahi-client/lookup.h>
#include <avahi-qt4/qt-watch.h>
#include <avahi-common/error.h>
#include <avahi-common/malloc.h>

#include <QObject>
#include <QStringList>
#include <QHostAddress>
#include <QMetaType>
#include <QAbstractListModel>

class QAvahiServiceBrowserModel : public QAbstractListModel
{
    Q_OBJECT
public:
    QAvahiServiceBrowserModel(QObject *parent = 0);
    ~QAvahiServiceBrowserModel();

    enum Option {
        NoOptions = 0,
        HideLocal = 1,
        HideIPv4 = 2,
        HideIPv6 = 4,
        HideSameLocalClient = 8
    };
    Q_DECLARE_FLAGS(Options, Option)

    enum CustomRoles {
        NameRole = Qt::UserRole + 1,
        TypeRole,
        DomainRole,
        ProtocolRole,
        InterfaceRole,
        HostNameRole,
        AddressRole,
        PortRole,
        TxtRole
    };

    void setAutoResolve(bool ar) { m_autoResolve = ar; }
    bool autoResolve() const { return m_autoResolve; }

    struct Service {
        Service() : protocol(-1), interface(-1), port(0), flags(0), resolved(false), resolver(0) { }

        QString name;
        QString type;
        QString domain;
        AvahiProtocol protocol;
        AvahiIfIndex interface;

        // valid only for resolved entries
        QString hostName;
        QHostAddress address;
        qint32 port;
        QStringList textRecords;
        int flags;
        bool isCached() const { return flags & AVAHI_LOOKUP_RESULT_CACHED; }
        bool isLocal() const { return flags & AVAHI_LOOKUP_RESULT_LOCAL; }
        bool isFromMulticastDns() const { return flags & AVAHI_LOOKUP_RESULT_MULTICAST; }
        bool isSameLocalClient() const { return flags & AVAHI_LOOKUP_RESULT_OUR_OWN; }
        bool isStatic() const { return flags & AVAHI_LOOKUP_RESULT_STATIC; }
        bool isFromWanDns() const { return flags & AVAHI_LOOKUP_RESULT_WIDE_AREA; }

        bool resolved;
        bool isValid() const { return protocol != -1 && interface != -1 && !address.isNull() && port != 0; }

        bool operator==(const Service &other) const {
            return name == other.name && type == other.type && domain == other.domain
                   && protocol == other.protocol && interface == other.interface
                   && hostName == other.hostName && address == other.address
                   && port == other.port && textRecords == other.textRecords
                   && flags == other.flags;
        }

        private:
            AvahiServiceResolver *resolver;
            friend class QAvahiServiceBrowserModel;
    };

    int error() const { return m_error; }
    QString errorString() const { return m_errorString; }

    enum Notification {
        Error,
        ClientConnecting,
        ServerRunning,
        ServerNameCollision,
        ServerRegistering,
    };

    Service serviceFromIndex(const QModelIndex &idx) const { return m_services.value(idx.row()); }

    void browse(const QString &serviceType, Options options = 0);

    // reimp
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

public slots:
    void resolve(int idx);
    void resolve(const QModelIndex &idx);

signals:
    void changeNotification(Notification notification);
    void done();
    void resolved(int idx);

private:
    static void client_callback(AvahiClient *c, AvahiClientState state, void *userdata);
    void clientCallback(AvahiClient *client, AvahiClientState state);

    static void browser_callback(AvahiServiceBrowser *browser, AvahiIfIndex interface, AvahiProtocol protocol, AvahiBrowserEvent event, const char *name, const char *type, const char *domain, AvahiLookupResultFlags flags, void *userdata);
    void browserCallback(AvahiServiceBrowser *browser, AvahiIfIndex interface, AvahiProtocol protocol, AvahiBrowserEvent event, const char *name, const char *type, const char *domain, AvahiLookupResultFlags flags);

    static void resolver_callback(AvahiServiceResolver *r, AvahiIfIndex interface, AvahiProtocol protocol, AvahiResolverEvent event,
                                  const char *name, const char *type, const char *domain, const char *host_name, const AvahiAddress *address,
                                  uint16_t port, AvahiStringList *txt, AvahiLookupResultFlags flags, void* userdata);
    void resolverCallback(AvahiServiceResolver *r, AvahiIfIndex interface, AvahiProtocol protocol, AvahiResolverEvent event,
                          const char *name, const char *type, const char *domain, const char *host_name, const AvahiAddress *address,
                          uint16_t port, AvahiStringList *txt, AvahiLookupResultFlags flags);
    
    void initialize();
    void uninitialize();
    int serviceIndex(const char *name, const char *type, const char *domain, AvahiIfIndex interface, AvahiProtocol protocol);
    void doBrowse(AvahiClient *client);
    void resetModel();
    void resolve(Service *service);

    AvahiClient *m_client;
    QString m_serviceType;
    Options m_options;
    enum BrowseType { NoBrowserType, ServiceBrowser, TypeBrowser } m_browserType;
    void *m_browser;
    int m_error;
    QString m_errorString;
    bool m_autoResolve;
    QList<Service> m_services;
    QList<int> m_rowToServiceIndex;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QAvahiServiceBrowserModel::Options)
Q_DECLARE_METATYPE(QAvahiServiceBrowserModel::Service)

#endif // QAVAHISERVICEBROWSERMODEL_H

