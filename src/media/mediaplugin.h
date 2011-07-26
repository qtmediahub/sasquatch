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

public:
    enum PluginRole { Undefined, Unadvertized, Music, Video, Picture, Dashboard, Weather, SingletonRoles, Store, Web, Application, Game, Map, Radio, RoleCount };

    MediaPlugin(const QString &name, PluginRole role, QObject *parent = 0)
        : QObject(parent), m_name(name), m_role(role)
    {}

    virtual ~MediaPlugin() { /*no impl*/ }

    QString name() const { return m_name; }
    PluginRole role() const { return m_role; }

private:
    QString m_name;
    PluginRole m_role;
};

Q_DECLARE_INTERFACE(MediaPlugin, "com.qtmediahub.MediaPlugin/1.0")

#endif // MEDIAPLUGIN_H
