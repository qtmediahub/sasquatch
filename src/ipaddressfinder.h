#ifndef IPADDRESSFINDER
#define IPADDRESSFINDER

#include <QtDeclarative>

class QProcess;

class IpAddressFinder : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QStringList ipAddresses READ getIpAddresses NOTIFY ipAddressesChanged)

public:
    IpAddressFinder(QObject *parent = 0);
    QStringList getIpAddresses() { return ipAddressesList; }

signals:
    void ipAddressesChanged();

public slots:
    void updateIpAddresses();
    void startProcess();

private:
    QProcess* p;
    bool restricted;
    QStringList ipAddressesList;
};

QML_DECLARE_TYPE(IpAddressFinder)


#endif // IPADDRESSFINDER
