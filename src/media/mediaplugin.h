#ifndef MEDIAPLUGIN_H
#define MEDIAPLUGIN_H

#include <QObject>
#include <QStringList>
#include <QMetaEnum>
#include "global.h"

class QDeclarativeContext;

class QMH_EXPORT MediaPlugin : public QObject
{
    Q_OBJECT
    Q_ENUMS(PluginRole)
    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(PluginRole role READ role CONSTANT)
    Q_PROPERTY(QObject* visualElement READ visualElement CONSTANT)

public:
    enum PluginRole { Undefined, Unadvertized, Music, Video, Picture, Dashboard, Weather, SingletonRoles, Store, Web, Application, Game, Map, Radio, RoleCount };

    MediaPlugin(const QString &name, PluginRole role, QObject *parent = 0)
        : QObject(parent), m_name(name), m_role(role)
    {}

    virtual ~MediaPlugin() { /*no impl*/ }

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

Q_DECLARE_INTERFACE(MediaPlugin, "com.qtmediahub.MediaPlugin/1.0")

#endif // MEDIAPLUGIN_H
