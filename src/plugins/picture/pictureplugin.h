#ifndef PICTUREPLUGIN_H
#define PICTUREPLUGIN_H

#include <QObject>

#include "qmhplugininterface.h"

class PicturePlugin : public QObject, public QMHPluginInterface
{
    Q_OBJECT
    Q_INTERFACES(QMHPluginInterface)

public:
    ~PicturePlugin() {}
    QString name() const { return tr("Pictures"); }
    bool browseable() const { return true; }
    QString role() const { return "picture"; }
};

#endif // PICTUREPLUGIN_H
