#ifndef QMHPLUGININTERFACE_H
#define QMHPLUGININTERFACE_H

class QMHPluginInterface
{
public:
    virtual ~QMHPluginInterface() { /* */ }
    virtual QString name() const = 0;
    //Fixme: roles should be a constrained enumeration
    virtual QString role() const = 0;
    virtual bool browseable() const { return false; }
};

Q_DECLARE_INTERFACE(QMHPluginInterface, "com.nokia.QMH.PluginInterface/1.0")

#endif // QMHPLUGININTERFACE_H
