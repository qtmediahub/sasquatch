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
    mActionList << "Files"
                << "Addons"
                << "Library";

    m_model = new MusicModel(this);

    mName = tr("Music");
    mRole = Music;
}

QObject *MusicPlugin::pluginProperties() const
{
    return const_cast<MusicPlugin *>(this);
}

void MusicPlugin::registerPlugin(QDeclarativeContext *context)
{
    //FIXME: these structures should be suitably disconnected from QML to be usable from html
    //Post CES adjustment :)
    if(context) {
        context->engine()->addImageProvider("musicmodel", new MusicModelImageProvider);
    }
}

Q_EXPORT_PLUGIN2(music, MusicPlugin)
