#include "videoplugin.h"

#include <QtPlugin>

#include "submenuentry.h"

VideoPlugin::VideoPlugin()
{
    for(int i = 0; i < 10; ++i) {
        SubMenuEntry *tmp = new SubMenuEntry(this);
        tmp->setName("Entry" + QString::number(i));
        mChildItems << tmp;
    }
}

Q_EXPORT_PLUGIN2(video, VideoPlugin)
