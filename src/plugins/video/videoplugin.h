#ifndef VIDEOPLUGIN_H
#define VIDEOPLUGIN_H

#include <QObject>
#include <QDeclarativeContext>
#include "qmhplugin.h"

class VideoPlugin : public QMHPlugin
{
    Q_OBJECT
    Q_INTERFACES(QMHPlugin)

public:
    VideoPlugin();
    ~VideoPlugin() {}
};

#endif // VIDEOPLUGIN_H
