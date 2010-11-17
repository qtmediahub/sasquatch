#ifndef VIDEOPLUGIN_H
#define VIDEOPLUGIN_H

#include <QObject>

#include "qmhplugininterface.h"

class VideoPlugin : public QObject, public QMHPluginInterface
{
    Q_OBJECT
    Q_INTERFACES(QMHPluginInterface)

public:
    ~VideoPlugin() {}
    QString name() const { return tr("Videos"); }
    bool browseable() const { return true; }
    QString role() const { return "videos"; }
};

#endif // VIDEOPLUGIN_H
