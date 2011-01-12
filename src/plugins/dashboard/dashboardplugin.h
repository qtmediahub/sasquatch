#ifndef DASHBOARDPLUGIN_H
#define DASHBOARDPLUGIN_H

#include <QObject>

#include "qmhplugininterface.h"
#include "dashboard.h"

class DashboardPlugin : public QObject, public QMHPluginInterface
{
    Q_OBJECT
    Q_INTERFACES(QMHPluginInterface)

public:
    DashboardPlugin()
    {
        setName(tr("Dashboard"));
        setRole("dashboard");
    }

    ~DashboardPlugin() {}

    QList<QObject*> childItems() const { return mChildItems; }

    void registerPlugin(QDeclarativeContext *context) { Q_UNUSED(context); qmlRegisterType<Dashboard>("Dashboard", 1, 0, "Dashboard"); }
private:
    QList<QObject*> mChildItems;
};

#endif // DASHBOARDPLUGIN_H
