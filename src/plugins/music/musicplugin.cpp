#include "musicplugin.h"

#include <QtGui>
#include <QtDeclarative>

class Files : public QObject
{
    Q_OBJECT
public:
    Files(QObject *parent = 0) : QObject(parent) { }
};

MusicPlugin::MusicPlugin()
{
    setActionList(
                QStringList()
                << "Files"
                << "Addons"
                << "Library");

    m_model = new MusicModel(this);

    setName(tr("Music"));
    setRole("music");
}

QObject *MusicPlugin::pluginProperties() const
{
    return const_cast<MusicPlugin *>(this);
}

void MusicPlugin::registerPlugin(QDeclarativeContext *context)
{
    //FIXME: these structures should be suitably disconnected from QML to be usable from html
    //Post CES adjustment :)
    if(context)
        m_model->registerImageProvider(context);
}

Q_EXPORT_PLUGIN2(music, MusicPlugin)
