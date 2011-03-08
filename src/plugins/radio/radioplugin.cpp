#include "radioplugin.h"

#include <QtPlugin>

RadioPlugin::RadioPlugin()
{
    setActionList(
                QStringList()
                << "Files"
                << "Addons"
                << "Library");

    m_model = new RadioModel(this);

    setName(tr("Radio"));
    setRole("radio");
}

QObject *RadioPlugin::pluginProperties() const
{
    return const_cast<RadioPlugin *>(this);
}

void RadioPlugin::registerPlugin(QDeclarativeContext *context)
{
    //FIXME: these structures should be suitably disconnected from QML to be usable from html
    //Post CES adjustment :)
    if (context)
        m_model->registerImageProvider(context);
}

Q_EXPORT_PLUGIN2(radio, RadioPlugin)
