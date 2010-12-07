#include "pictureplugin.h"

#include <QtPlugin>

PicturePlugin::PicturePlugin()
{
    m_model = new PictureModel(this);
}

QObject *PicturePlugin::pluginProperties() const
{
    return const_cast<PicturePlugin *>(this);
}

void PicturePlugin::registerPlugin(QDeclarativeContext *context)
{
    //FIXME: these structures should be suitably disconnected from QML to be usable from html
    //Post CES adjustment :)
    if (context)
        m_model->registerImageProvider(context);
}

Q_EXPORT_PLUGIN2(picture, PicturePlugin)
