#ifndef QMHPLUGININTERFACE_H
#define QMHPLUGININTERFACE_H

#include <QObject>
#include <QStringList>

class QMHPluginInterface
{
public:
    virtual ~QMHPluginInterface() { /* */ }
    virtual QString name() const { return "Uninitialized plugin name"; }
    //Fixme: roles should be a constrained enumeration
    virtual QString role() const { return "Undefined"; }
    //Please note this property indicates that an items models can be populated
    virtual bool browseable() const { return false; }
    virtual QList<QObject*> childItems() const { return QList<QObject*>(); }

    virtual QObject* visualElement() const { return 0; }
    virtual void setVisualElement(QObject *element) { Q_UNUSED(element) }

    virtual QStringList visualElementProperties() const { return QStringList(); }
    virtual void setVisualElementProperties(const QStringList& properties) { Q_UNUSED(properties); }

    //Only make sense for skin specific plugins
    virtual void setName(const QString &name) { Q_UNUSED(name) }
    virtual void setBrowseable(bool browseable) { Q_UNUSED(browseable) }
    virtual void setRole(const QString &role) { Q_UNUSED(role) }
};

Q_DECLARE_INTERFACE(QMHPluginInterface, "com.nokia.QMH.PluginInterface/1.0")

#endif // QMHPLUGININTERFACE_H
