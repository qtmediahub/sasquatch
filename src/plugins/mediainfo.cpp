#include "mediainfo.h"
#include "backend.h"

MediaInfo::MediaInfo(MediaModel::MediaInfoType type, const QString &path)
    : parent(0)
    , type(type)
    , filePath(path)
    , status(NotSearched)
    , fileSize(-1)
{
    if (type == MediaModel::Directory || type == MediaModel::SearchPath) {
        //FIXME: This should be path/../ ??
        MediaInfo *info = new MediaInfo(MediaModel::DotDot, path + "/../");
        info->name = QT_TRANSLATE_NOOP("MediaModel", "..");
        info->parent = this;
        children.append(info);
    }
    if(!filePath.isEmpty())
    {
        QFileInfo fo(filePath);
        name = fo.fileName();
        fileSize = fo.size();
        fileDateTime = fo.created();
        hash = QCryptographicHash::hash(QString("file://" + fo.absoluteFilePath()).toUtf8(), QCryptographicHash::Md5).toHex();
        thumbnailPath = Backend::instance()->thumbnailPath() + hash + ".png";
    }
}

MediaInfo::~MediaInfo()
{
    qDeleteAll(children);
    children.clear();
}
