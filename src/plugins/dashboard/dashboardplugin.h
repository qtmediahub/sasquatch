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
    ~DashboardPlugin() {}
    QString name() const { return tr("Dashboard"); }
    QString role() const { return "dashboard"; }
    QList<QObject*> childItems() const { return mChildItems; }

    QObject* visualElement() const { return mVisualElement; }
    void setVisualElement(QObject *element) { mVisualElement = element; }

    void registerPlugin(QDeclarativeContext *context) { Q_UNUSED(context); qmlRegisterType<Dashboard>("Dashboard", 1, 0, "Dashboard"); }
private:
    QList<QObject*> mChildItems;
    QObject *mVisualElement;
};

#endif // DASHBOARDPLUGIN_H
