#include "media.h"

namespace Media {

QHash<int, QByteArray> roleNames()
{
    QHash<int, QByteArray> hash;
    hash[FilePathRole] = "filePath";
    hash[PreviewUrlRole] = "previewUrl";
    hash[ModelIndexRole] = "modelIndex";
    return hash;
}

} // namespace Media

