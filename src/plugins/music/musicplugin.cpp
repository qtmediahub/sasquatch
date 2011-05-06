#include "musicplugin.h"

#include <QtGui>
#include <QtDeclarative>

MusicPlugin::MusicPlugin()
    : QMHPlugin(tr("Music"), Music)
{
    m_model = new MusicModel(this);
}

void MusicPlugin::registerPlugin(QDeclarativeContext *context)
{
    if (context) {
        context->engine()->addImageProvider("musicmodel", new MusicModelImageProvider);
    }
}

Q_EXPORT_PLUGIN2(music, MusicPlugin)
