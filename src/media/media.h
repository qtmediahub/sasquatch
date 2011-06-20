#ifndef MEDIA_H
#define MEDIA_H

#include <QtDeclarative>
#include "global.h"

class QMH_EXPORT Media : public QObject
{
    Q_OBJECT
    Q_ENUMS(StandardRoles)

public:
    enum StandardRoles {
        FilePathRole = Qt::UserRole + 1,
        PreviewUrlRole,
        ModelIndexRole,
        TitleRole
    };

    static QHash<int, QByteArray> roleNames();

private:
    Media() { }
};

QML_DECLARE_TYPE(Media)

#endif // MEDIA_H

