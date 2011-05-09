#ifndef QMHPLUGIN_H
#define QMHPLUGIN_H

#include <QObject>
#include <QStringList>
#include <QMetaEnum>
#include "global.h"

class QDeclarativeContext;

class QMH_EXPORT QMHPlugin : public QObject
{
    Q_OBJECT
    Q_ENUMS(PluginRole)
    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(PluginRole role READ role CONSTANT)
    Q_PROPERTY(QObject* visualElement READ visualElement CONSTANT)

public:
    enum PluginRole { Undefined, Unadvertized, Music, Video, Picture, Dashboard, Weather, SingletonRoles, Store, Web, Application, Game, Map, RoleCount };

    QMHPlugin(const QString &name, PluginRole role, QObject *parent = 0)
        : QObject(parent), m_name(name), m_role(role)
    {}

    virtual ~QMHPlugin() { /*no impl*/ }

    QString name() const { return m_name; }
    PluginRole role() const { return m_role; }
    virtual QObject* visualElement() const { return 0; }

    //These plugins should be equally usable from html
    virtual void registerPlugin(QDeclarativeContext *context = 0) { Q_UNUSED(context); }
    virtual void unregisterPlugin() { /*no impl*/ }

    virtual bool dependenciesSatisfied() const { return true; }

private:
    QString m_name;
    PluginRole m_role;
};

Q_DECLARE_INTERFACE(QMHPlugin, "com.nokia.QMH.Plugin/1.0")

#endif // QMHPLUGIN_H
