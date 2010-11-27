#ifndef DASHBOARDPLUGIN_H
#define DASHBOARDPLUGIN_H

#include <QObject>

#include "qmhplugin.h"
#include "dashboard.h"

class DashboardPlugin : public QMHPlugin
{
    Q_OBJECT
    Q_INTERFACES(QMHPlugin)

public:
    DashboardPlugin() {
        mName = tr("Dashboard");
        mBrowseable = true;
        mRole = "dashboard";
    }
    ~DashboardPlugin() {}

    void registerPlugin() { 
        qmlRegisterType<Dashboard>("Dashboard", 1, 0, "Dashboard"); 
        QMHPlugin::registerPlugin();
    }
};

#endif // DASHBOARDPLUGIN_H
