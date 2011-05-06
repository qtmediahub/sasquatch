#include "pictureplugin.h"

#include <QtPlugin>

PicturePlugin::PicturePlugin()
    : QMHPlugin(tr("Pictures"), Picture)
{
    m_model = new PictureModel(this);
}

void PicturePlugin::registerPlugin(QDeclarativeContext *context)
{
    if (context)
        context->engine()->addImageProvider("picturemodel", new PictureModelImageProvider);
}

Q_EXPORT_PLUGIN2(picture, PicturePlugin)
