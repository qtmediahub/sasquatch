#include "ipaddressfinder.h"

#include <QProcess>
#include <QDebug>
#include <QHostAddress>

IpAddressFinder::IpAddressFinder(QObject *parent)
    : QObject(parent)
{
    restricted = true;

    p = new QProcess(this);
    connect(p, SIGNAL(readyReadStandardOutput()), this, SLOT(updateIpAddresses()));
    startProcess();

    QTimer* t = new QTimer(this);
    connect(t, SIGNAL(timeout()), this, SLOT(startProcess()));
    t->setInterval(5000);
    t->start();
}

void IpAddressFinder::startProcess()
{
    if(p->state() == QProcess::NotRunning)
        p->start("/sbin/ifconfig");
    else
        qWarning() << "WARNING: IpAddressFinder::startProcess() was still running ... something is wrong on you system!";
}


void IpAddressFinder::updateIpAddresses()
{
    QString pResults(p->readAll());
    QStringList sl = pResults.split(QRegExp("\\s|:|\\n|\\t"));

    QHostAddress ha;
    bool foundNew = false;
    bool lastOneWas_addr_or_inet = false;

    foreach (const QString &str, sl) {

        if (ha.setAddress(str)
                && !ipAddressesList.contains(str)
                && (!restricted || lastOneWas_addr_or_inet)
                && !str.startsWith("127"))
        {
            foundNew = true;
            ipAddressesList.append(str);
            qDebug() << "New ip address found:" << str;

            lastOneWas_addr_or_inet = false;
        } else {

            if (str.compare("inet", Qt::CaseInsensitive)==0 || str.compare("addr", Qt::CaseInsensitive)==0)
                lastOneWas_addr_or_inet = true;
            else
                lastOneWas_addr_or_inet = false;
        }
    }

    if (foundNew)
        emit ipAddressesChanged();
}

