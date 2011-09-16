#ifndef SKINMANAGER_H
#define SKINMANAGER_H

#include <QtCore>
#include <QHash>
#include "global.h"
#include "skin.h"

class TarFileEngineHandler;

class QMH_EXPORT SkinManager : public QObject
{
	Q_OBJECT
public:
    SkinManager(QObject *parent = 0);
    ~SkinManager();

    QHash<QString, Skin *> skins() const;

private slots:
    void discoverSkins();
    void handleDirChanged(const QString &dir);

private:
    QFileSystemWatcher m_pathMonitor;
    TarFileEngineHandler *m_tarFileEngineHandler;
    QHash<QString, Skin *> m_skins;
};

#endif // SKINMANAGER_H


