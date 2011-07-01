#ifndef PICTUREPLUGIN_H
#define PICTUREPLUGIN_H

#include <QObject>

#include "qmhplugin.h"

class PicturePlugin : public QMHPlugin
{
    Q_OBJECT
    Q_INTERFACES(QMHPlugin)

public:
    PicturePlugin();
    ~PicturePlugin() {}
};

#endif // PICTUREPLUGIN_H
