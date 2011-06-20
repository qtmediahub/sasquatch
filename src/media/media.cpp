#include "media.h"

QHash<int, QByteArray> Media::roleNames()
{
    QHash<int, QByteArray> hash;
    hash[FilePathRole] = "filePath";
    hash[PreviewUrlRole] = "previewUrl";
    hash[ModelIndexRole] = "modelIndex";
    hash[TitleRole] = "title";
    return hash;
}

