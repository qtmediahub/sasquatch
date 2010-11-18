#ifndef QMHPLUGININTERFACE_H
#define QMHPLUGININTERFACE_H

class QMHPluginInterface
{
public:
    virtual ~QMHPluginInterface() { /* */ }
    virtual QString name() const = 0;
    //Fixme: roles should be a constrained enumeration
    virtual QString role() const = 0;
    //Please note this property indicates that an items models can be populated
    virtual bool browseable() const { return false; }

    //Only make sense for skin specific plugins
    virtual void setName(const QString &name) { Q_UNUSED(name) }
    virtual void setBrowseable(bool browseable) { Q_UNUSED(browseable) }
    virtual void setRole(const QString &role) { Q_UNUSED(role) }
};

Q_DECLARE_INTERFACE(QMHPluginInterface, "com.nokia.QMH.PluginInterface/1.0")

#endif // QMHPLUGININTERFACE_H
