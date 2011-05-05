#ifndef MEDIA_H
#define MEDIA_H

#include <QtDeclarative>

class Media : public QObject
{
    Q_OBJECT
    Q_ENUMS(StandardRoles)

public:
    enum StandardRoles {
        FilePathRole = Qt::UserRole + 1,
        PreviewUrlRole,
        ModelIndexRole
    };

    static QHash<int, QByteArray> roleNames();

private:
    Media() { }
};

QML_DECLARE_TYPE(Media)

#endif // MEDIA_H

