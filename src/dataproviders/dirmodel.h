#ifndef DIRMODEL_H
#define DIRMODEL_H

#include <QDirModel>
#include <QtDeclarative>

class DirModel : public QDirModel
{
    Q_OBJECT
public:
    DirModel(QObject *parent = 0) : QDirModel(parent)
    {
        setReadOnly(true);
        setFilter(QDir::AllDirs | QDir::NoDot);
    }
    Q_INVOKABLE QString filePath(const QModelIndex &index) {
        return index.isValid() ? index.data(QDirModel::FilePathRole).toString() : "/";
    }
    Q_INVOKABLE QString baseName(const QModelIndex &index) {
        return index.isValid() ? QFileInfo(index.data(QDirModel::FilePathRole).toString()).baseName() : "/";
    }

};

QML_DECLARE_TYPE(DirModel)

#endif // DIRMODEL_H

