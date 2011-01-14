#include "mediainfo.h"

MediaInfo::MediaInfo(MediaModel::MediaInfoType type)
    : parent(0)
    , type(type)
    , status(NotSearched)
    , fileSize(-1)
{
    if (type == MediaModel::Directory || type == MediaModel::SearchPath) {
        MediaInfo *info = new MediaInfo(MediaModel::DotDot);
        info->name = QT_TRANSLATE_NOOP("MediaModel", "..");
        info->parent = this;
        children.append(info);
    }
}

MediaInfo::~MediaInfo()
{
    qDeleteAll(children);
    children.clear();
}
