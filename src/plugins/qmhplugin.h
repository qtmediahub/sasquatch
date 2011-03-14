#ifndef QMHPLUGIN_H
#define QMHPLUGIN_H

#include <QObject>

#include "qmhplugininterface.h"

class GenericPlugin : public QObject, public QMHPluginInterface
{
    Q_OBJECT
    Q_INTERFACES(QMHPluginInterface)
public:
    GenericPlugin()
        : QObject(),
          QMHPluginInterface()
          { /* */ }
    ~GenericPlugin() {}

    QObject *pluginProperties() const { return const_cast<GenericPlugin*>(this); }
};

class QDeclarativeContext;

class QMHPlugin : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY pluginChanged)
    Q_PROPERTY(QString role READ role WRITE setRole NOTIFY pluginChanged)
    Q_PROPERTY(QObject* visualElement READ visualElement WRITE setVisualElement NOTIFY pluginChanged)
    Q_PROPERTY(QStringList visualElementProperties READ visualElementProperties WRITE setVisualElementProperties NOTIFY pluginChanged)
    Q_PROPERTY(QObject* actionMap READ actionMap WRITE setActionMap NOTIFY pluginChanged)
    Q_PROPERTY(QStringList actionList READ actionList WRITE setActionList NOTIFY pluginChanged)
    Q_PROPERTY(QObject* pluginProperties READ pluginProperties NOTIFY pluginChanged)

public:
    QMHPlugin(QMHPluginInterface *interface = new GenericPlugin(), QObject *parent = 0)
        : QObject(parent)
        , mInterface(interface)
    {}

    ~QMHPlugin() { delete mInterface; }

    QString name() const { return mInterface->name(); }
    void setName(const QString &name) { mInterface->setName(name); emit pluginChanged(); }

    QString role() const { return mInterface->role(); }
    void setRole(const QString &role) { mInterface->setRole(role); emit pluginChanged(); }

    bool advertized() const { return mInterface->advertized(); }
    void setAdvertized(bool advertized) { mInterface->setAdvertized(advertized); emit pluginChanged(); }

    QObject* visualElement() const { return mInterface->visualElement(); }
    void setVisualElement(QObject *element) { mInterface->setVisualElement(element); emit pluginChanged(); }

    QStringList visualElementProperties() const { return mInterface->visualElementProperties(); }
    void setVisualElementProperties(const QStringList& properties) { mInterface->setVisualElementProperties(properties); emit pluginChanged(); }

    QObject* actionMap() const { return mInterface->actionMap(); }
    void setActionMap(QObject *map) { mInterface->setActionMap(map); emit pluginChanged(); }

    QStringList actionList() const { return mInterface->actionList(); }
    void setActionList(const QStringList& actions) { mInterface->setActionList(actions); emit pluginChanged(); }

    //These plugins should be equally usable from html
    void registerPlugin(QDeclarativeContext *context = 0) { mInterface->registerPlugin(context); }
    void unregisterPlugin() { mInterface->unregisterPlugin(); }

    bool dependenciesSatisfied() const { return mInterface->dependenciesSatisfied(); }

    virtual QObject *pluginProperties() const { return mInterface->pluginProperties(); }

signals:
    void pluginChanged();

private:
    QMHPluginInterface *mInterface;
};

#endif // QMHPLUGIN_H
