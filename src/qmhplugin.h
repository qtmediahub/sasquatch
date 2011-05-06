#ifndef QMHPLUGIN_H
#define QMHPLUGIN_H

#include <QObject>
#include <QStringList>
#include <QMetaEnum>

class QDeclarativeContext;

class QMHPlugin : public QObject
{
    Q_OBJECT
    Q_ENUMS(PluginRole)
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY pluginChanged)
    Q_PROPERTY(PluginRole role READ role WRITE setRole NOTIFY pluginChanged)
    Q_PROPERTY(QObject* visualElement READ visualElement WRITE setVisualElement NOTIFY pluginChanged)

public:
    enum PluginRole { Undefined, Unadvertized, Music, Video, Picture, Dashboard, Weather, SingletonRoles, Store, Web, Application, Game, Map, RoleCount };

    QMHPlugin(QObject *parent = 0)
        : QObject(parent),
          mRole(Undefined),
          mVisualElement(0),
          mActionMap(0)
    {}

    virtual ~QMHPlugin() { /*no impl*/ }

    QString name() const { return mName; }
    void setName(const QString &name) { mName = name; emit pluginChanged(); }

    PluginRole role() const { return mRole; }
    void setRole(PluginRole role) { mRole = role; emit pluginChanged(); }

    QObject* visualElement() const { return mVisualElement; }
    void setVisualElement(QObject *element) {
        mVisualElement = element;
        emit visualElementChanged(this);
        emit pluginChanged();
    }

    //These plugins should be equally usable from html
    virtual void registerPlugin(QDeclarativeContext *context = 0) { Q_UNUSED(context); }
    virtual void unregisterPlugin() { /*no impl*/ }

    virtual bool dependenciesSatisfied() const { return true; }

signals:
    void pluginChanged();
    void visualElementChanged(QMHPlugin *plugin);

protected:
    QString mName;
    PluginRole mRole;
    QObject *mVisualElement;
    QObject *mActionMap;
    QStringList mActionList;
};

Q_DECLARE_INTERFACE(QMHPlugin, "com.nokia.QMH.Plugin/1.0")

#endif // QMHPLUGIN_H
