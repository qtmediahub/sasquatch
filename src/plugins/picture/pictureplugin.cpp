#include "pictureplugin.h"
#include "pictureparser.h"

#include <QtPlugin>

PicturePlugin::PicturePlugin()
    : QMHPlugin(tr("Pictures"), Picture)
{
    PictureParser *parser = new PictureParser;
    QMetaObject::invokeMethod(MediaScanner::instance(), "addParser", Qt::QueuedConnection, 
                              Q_ARG(MediaParser *, parser));
}

Q_EXPORT_PLUGIN2(picture, PicturePlugin)
