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
    QMLModuleDiscoveryPlugin() : QMHPlugin(tr("Module Discovery"), Unadvertized) {
        qmlRegisterType<QMLModuleDiscovery>("QMLModuleDiscovery", 1, 0, "QMLModuleDiscovery");
    }
    ~QMLModuleDiscoveryPlugin() {}
};

#endif // QMLMODULEDISCOVERYPLUGIN_H
