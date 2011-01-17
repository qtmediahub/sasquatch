#ifndef QMLMODULEDISCOVERYPLUGIN_H
#define QMLMODULEDISCOVERYPLUGIN_H

#include <QObject>

#include "qmhplugininterface.h"
#include "qmlmodulediscovery.h"

class QMLModuleDiscoveryPlugin : public QObject, public QMHPluginInterface
{
    Q_OBJECT
    Q_INTERFACES(QMHPluginInterface)

public:
    QMLModuleDiscoveryPlugin() { setRole(""); }
    ~QMLModuleDiscoveryPlugin() {}

    QList<QObject*> childItems() const { return mChildItems; }

    void registerPlugin(QDeclarativeContext *context) { Q_UNUSED(context); qmlRegisterType<QMLModuleDiscovery>("QMLModuleDiscovery", 1, 0, "QMLModuleDiscovery"); }
private:
    QList<QObject*> mChildItems;
};

#endif // QMLMODULEDISCOVERYPLUGIN_H
