#ifndef QMLMODULEDISCOVERYPLUGIN_H
#define QMLMODULEDISCOVERYPLUGIN_H

#include <QObject>

#include "qmhplugin.h"
#include "qmlmodulediscovery.h"

class QMLModuleDiscoveryPlugin : public QMHPlugin
{
    Q_OBJECT
    Q_INTERFACES(QMHPlugin)

public:
    QMLModuleDiscoveryPlugin() {}
    ~QMLModuleDiscoveryPlugin() {}

    QList<QObject*> childItems() const { return mChildItems; }

    void registerPlugin(QDeclarativeContext *context) { Q_UNUSED(context); qmlRegisterType<QMLModuleDiscovery>("QMLModuleDiscovery", 1, 0, "QMLModuleDiscovery"); }
private:
    QList<QObject*> mChildItems;
};

#endif // QMLMODULEDISCOVERYPLUGIN_H
