#ifndef QMLMODULEDISCOVERY_H
#define QMLMODULEDISCOVERY_H

#include <QObject>
#include <QDir>
#include <QDebug>

#include <QDeclarativeItem>

class QMLModuleDiscovery : public QDeclarativeItem
{
    Q_OBJECT
    Q_PROPERTY(QString path READ path WRITE setPath)
    Q_PROPERTY(QStringList modules READ modules)

public:
    explicit QMLModuleDiscovery(QDeclarativeItem *parent = 0) : QDeclarativeItem(parent) { /**/ }
    QString path() const { return mPath; }
    void setPath(const QString &path) { mPath = path; searchPath(); }
    QStringList modules() const { return mAvailableItems; }
private:
    void searchPath();
    QStringList mAvailableItems;
    QString mPath;
};

#endif // QMLMODULEDISCOVERY_H
