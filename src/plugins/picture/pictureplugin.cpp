#include "pictureplugin.h"
#include "pictureparser.h"

#include <QtPlugin>

PicturePlugin::PicturePlugin()
    : QMHPlugin(tr("Pictures"), Picture)
{
    PictureParser *parser = new PictureParser;
    MediaScanner::instance()->addParser(parser);
}

Q_EXPORT_PLUGIN2(picture, PicturePlugin)
