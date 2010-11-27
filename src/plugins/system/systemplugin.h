#ifndef SYSTEMPLUGIN_H
#define SYSTEMPLUGIN_H

#include <QObject>

#include "qmhplugin.h"

class SystemPlugin : public QMHPlugin
{
    Q_OBJECT
    Q_INTERFACES(QMHPlugin)

public:
    SystemPlugin() {
        mName = tr("System");
        mBrowseable = false;
        mRole = "system";
    }
    ~SystemPlugin() {}
};

#endif // SYSTEMPLUGIN_H
