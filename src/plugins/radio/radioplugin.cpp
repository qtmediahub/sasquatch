#include "radioplugin.h"

#include <QtPlugin>

RadioPlugin::RadioPlugin()
    : QMHPlugin(tr("Radio"), Application)
{
    m_model = new RadioModel(this);
}

void RadioPlugin::registerPlugin(QDeclarativeContext *context)
{
    if (context)
        m_model->registerImageProvider(context);
}

Q_EXPORT_PLUGIN2(radio, RadioPlugin)
