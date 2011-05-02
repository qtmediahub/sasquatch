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
    QMLModuleDiscoveryPlugin() { mRole = Unadvertized; }
    ~QMLModuleDiscoveryPlugin() {}

    void registerPlugin(QDeclarativeContext *context) { Q_UNUSED(context); qmlRegisterType<QMLModuleDiscovery>("QMLModuleDiscovery", 1, 0, "QMLModuleDiscovery"); }
};

#endif // QMLMODULEDISCOVERYPLUGIN_H
