#ifndef MEDIA_H
#define MEDIA_H

#include <QAbstractItemModel>

namespace Media {

enum StandardRoles {
    FilePathRole = Qt::UserRole + 1,
    PreviewUrlRole
};

QHash<int, QByteArray> roleNames();

} // namespace media

#endif // MEDIA_H

