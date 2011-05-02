#include "pictureplugin.h"

#include <QtPlugin>

PicturePlugin::PicturePlugin()
{
    mActionList << "Files"
                << "Addons"
                << "Library";

    m_model = new PictureModel(this);

    mName = tr("Pictures");
    mRole = Picture;
}

void PicturePlugin::registerPlugin(QDeclarativeContext *context)
{
    //FIXME: these structures should be suitably disconnected from QML to be usable from html
    //Post CES adjustment :)
    if (context)
        m_model->registerImageProvider(context);
}

Q_EXPORT_PLUGIN2(picture, PicturePlugin)
