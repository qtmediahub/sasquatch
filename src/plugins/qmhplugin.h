#ifndef QMHPLUGIN_H
#define QMHPLUGIN_H

#include <QObject>

#include "qmhplugininterface.h"

class GenericPlugin : public QObject, public QMHPluginInterface
{
    Q_OBJECT
    Q_INTERFACES(QMHPluginInterface)

public:
    ~GenericPlugin() {}
    QString name() const { return tr("Skin specific plugin"); }
    bool browseable() const { return false; }
    QString role() const { return "generic"; }
};

class QMHPlugin : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name NOTIFY pluginChanged)
    Q_PROPERTY(bool browseable READ browseable NOTIFY pluginChanged)
    Q_PROPERTY(QString role READ role NOTIFY pluginChanged)
public:
    QMHPlugin(QMHPluginInterface *interface = new GenericPlugin(), QObject *parent = 0)
        : QObject(parent)
        , mInterface(interface)
    {}

    QString name() const { return mInterface->name(); }
    bool browseable() const { return mInterface->browseable(); }
    QString role() const { return mInterface->role(); }

signals:
    void pluginChanged();

private:
    QMHPluginInterface *mInterface;
}
;

#endif // QMHPLUGIN_H
