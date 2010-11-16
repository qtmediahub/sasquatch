#ifndef QMHPLUGININTERFACE_H
#define QMHPLUGININTERFACE_H

#include <QObject>

class QMHPluginInterface : public QObject
{
    //Kills plugin capabilities
    //Q_OBJECT
    //Would be nice to enforce properties etc here
    Q_PROPERTY(QString name READ name)
    Q_PROPERTY(bool browseable READ browseable)
public:
    virtual ~QMHPluginInterface() { /* */ }
    virtual QString name() const = 0;
    virtual bool browseable() const { return false; }
};

Q_DECLARE_INTERFACE(QMHPluginInterface, "com.nokia.QMH.PluginInterface/1.0")

#endif // QMHPLUGININTERFACE_H
