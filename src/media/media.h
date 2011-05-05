#ifndef MEDIA_H
#define MEDIA_H

#include <QAbstractItemModel>

namespace Media {

enum StandardRoles {
    FilePathRole = Qt::UserRole + 1,
    PreviewUrlRole,
    ModelIndexRole
};

QHash<int, QByteArray> roleNames();

} // namespace media

Q_DECLARE_METATYPE(QModelIndex)

#endif // MEDIA_H

