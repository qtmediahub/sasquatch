#ifndef QMHPLUGININTERFACE_H
#define QMHPLUGININTERFACE_H

#include <QObject>
#include <QStringList>

class QDeclarativeContext;

class QMHPluginInterface
{
public:
    QMHPluginInterface() : mVisualElement(0) { }
    virtual ~QMHPluginInterface() { /* */ }
    virtual QString name() const { return "Uninitialized plugin name"; }
    //Fixme: roles should be a constrained enumeration
    virtual QString role() const { return "Undefined"; }
    //Please note this property indicates that an items models can be populated
    virtual bool browseable() const { return false; }
    virtual QList<QObject*> childItems() const { return QList<QObject*>(); }

    virtual QObject* visualElement() const { return mVisualElement; }
    virtual void setVisualElement(QObject *element) { mVisualElement = element; }

    virtual QStringList visualElementProperties() const { return mVisualElementProperties; }
    virtual void setVisualElementProperties(const QStringList& properties) { mVisualElementProperties = properties; }

    virtual void registerPlugin(QDeclarativeContext *context) { Q_UNUSED(context); }
    virtual void unregisterPlugin() { /**/ }

    //Only make sense for skin specific plugins
    virtual void setName(const QString &name) { Q_UNUSED(name) }
    virtual void setBrowseable(bool browseable) { Q_UNUSED(browseable) }
    virtual void setRole(const QString &role) { Q_UNUSED(role) }

    virtual QObject *pluginProperties() const { return 0; }

protected:
    QObject *mVisualElement;
    QStringList mVisualElementProperties;
};

Q_DECLARE_INTERFACE(QMHPluginInterface, "com.nokia.QMH.PluginInterface/1.0")

#endif // QMHPLUGININTERFACE_H
