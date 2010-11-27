#ifndef PICTUREPLUGIN_H
#define PICTUREPLUGIN_H

#include <QObject>

#include "qmhplugin.h"

class PicturePlugin : public QMHPlugin
{
    Q_OBJECT
    Q_INTERFACES(QMHPlugin)

public:
    PicturePlugin() {
        mName = tr("Pictures");
        mBrowseable = true;
        mRole = "pictures";
    }
    ~PicturePlugin() {}
};

#endif // PICTUREPLUGIN_H
