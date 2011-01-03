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
          QMHPluginInterface(),
          mName("Skin Plugin"),
          mRole("undefined"),
          mVisualElement(0) { /* */ }
    ~GenericPlugin() {}
    
    QString name() const { return mName; }
    void setName(const QString &name) { mName = name; }

    QString role() const { return mRole; }
    void setRole(const QString &role) { mRole = role; }

    QObject* visualElement() const { return mVisualElement; }
    void setVisualElement(QObject *element) { mVisualElement = element; }

    QStringList visualElementProperties() const { return mVisualElementProperties; }
    void setVisualElementProperties(const QStringList& properties) {
        mVisualElementProperties = properties;
        //Must be property/value pairs
        if(mVisualElementProperties.size() % 2)
            mVisualElementProperties.removeLast();
    }

    QObject *pluginProperties() const { return const_cast<GenericPlugin*>(this); }

private:
    QString mName;
    QString mRole;
    QObject *mVisualElement;
    QStringList mVisualElementProperties;
};

class QDeclarativeContext;

class QMHPlugin : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY pluginChanged)
    Q_PROPERTY(QString role READ role WRITE setRole NOTIFY pluginChanged)
    Q_PROPERTY(QList<QObject*> childItems READ childItems NOTIFY pluginChanged)
    Q_PROPERTY(QObject* visualElement READ visualElement WRITE setVisualElement NOTIFY pluginChanged)
    Q_PROPERTY(QStringList visualElementProperties READ visualElementProperties WRITE setVisualElementProperties NOTIFY pluginChanged)
    Q_PROPERTY(QObject* pluginProperties READ pluginProperties NOTIFY pluginChanged)

public:
    QMHPlugin(QMHPluginInterface *interface = new GenericPlugin(), QObject *parent = 0)
        : QObject(parent)
        , mInterface(interface)
    {}

    ~QMHPlugin() { delete mInterface; }

    QString name() const { return mInterface->name(); }
    void setName(const QString &name) { mInterface->setName(name); }

    QString role() const { return mInterface->role(); }
    void setRole(const QString &role) { mInterface->setRole(role); }

    QList<QObject*> childItems() const { return mInterface->childItems(); }

    QObject* visualElement() const { return mInterface->visualElement(); }
    void setVisualElement(QObject *element) { mInterface->setVisualElement(element); }

    QStringList visualElementProperties() const { return mInterface->visualElementProperties(); }
    void setVisualElementProperties(const QStringList& properties) { mInterface->setVisualElementProperties(properties); }

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
