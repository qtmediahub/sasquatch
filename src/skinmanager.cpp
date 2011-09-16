#include "skinmanager.h"
#include "libraryinfo.h"
#include "tarfileengine.h"

SkinManager::SkinManager(QObject *parent)
	: QObject(parent)
{
	connect(&m_pathMonitor, SIGNAL(directoryChanged(QString)), this, SLOT(handleDirChanged(QString)));
    foreach (const QString &skinPath, LibraryInfo::skinPaths()) {
        if (QDir(skinPath).exists())
            m_pathMonitor.addPath(skinPath);
    }

    m_tarFileEngineHandler = new TarFileEngineHandler;
    discoverSkins();
}

SkinManager::~SkinManager()
{
	delete m_tarFileEngineHandler;
}

QHash<QString, Skin *> SkinManager::skins() const
{
	return m_skins;
}

void SkinManager::handleDirChanged(const QString &dir)
{
    if (LibraryInfo::skinPaths().contains(dir)) {
        qWarning() << "Changes in skin path, repopulating skins";
        discoverSkins();
    }
}

void SkinManager::discoverSkins()
{
    qDeleteAll(m_skins.values()); // FIXME: err, skin pointer is help by run-time, this will cause a crash
    m_skins.clear();

    foreach (const QString &skinPath, LibraryInfo::skinPaths()) {
        QStringList potentialm_skins = QDir(skinPath).entryList(QDir::Dirs | QDir::NoDotAndDotDot);

        foreach(const QString &currentPath, potentialm_skins) {
            const QString prospectivePath = skinPath % "/" % currentPath;
            if (Skin *skin = Skin::createSkin(prospectivePath, this))
                m_skins.insert(skin->name(), skin);
        }
    }

    if (m_skins.isEmpty()) {
        qWarning() << "No m_skins are found in your skin paths"<< endl \
                   << "If you don't intend to run this without m_skins"<< endl \
                   << "Please read the INSTALL doc available here:" \
                   << "http://gitorious.org/qtmediahub/qtmediahub-core/blobs/master/INSTALL" \
                   << "for further details";
    } else {
        QStringList sl;
        foreach(Skin *skin, m_skins)
            sl.append(skin->name());
        qDebug() << "Available m_skins:" << sl.join(",");
    }
}

