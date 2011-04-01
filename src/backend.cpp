/****************************************************************************

This file is part of the QtMediaHub project on http://www.gitorious.org.

Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).*
All rights reserved.

Contact:  Nokia Corporation (qt-info@nokia.com)**

You may use this file under the terms of the BSD license as follows:

"Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
* Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
* Neither the name of Nokia Corporation and its Subsidiary(-ies) nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."

****************************************************************************/

#include "backend.h"
#include "plugins/qmhplugin/qmhplugin.h"

#include "qmh-config.h"
#include "rpc/rpcconnection.h"
#include "skin.h"

#ifdef QMH_AVAHI
#include "qavahiservicebrowsermodel.h"
#endif

#include <QDir>
#include <QString>
#include <QPluginLoader>
#include <QCoreApplication>
#include <QVariant>
#include <QFileSystemModel>
#include <QDesktopServices>
#include <QFileSystemWatcher>

#ifdef GL
#include <QGLFormat>
#endif

#include <QDebug>

Backend* Backend::pSelf = 0;

class BackendPrivate : public QObject
{
    Q_OBJECT
public:
    BackendPrivate(Backend *p)
        : QObject(p),
      #ifdef Q_OS_MAC
          platformOffset("/../../.."),
      #endif
          basePath(QCoreApplication::applicationDirPath() + platformOffset),
          // Use "large" instead of appName to fit freedesktop spec
          thumbnailPath(Config::value("thumbnail-path", QDir::homePath() + "/.thumbnails/" + qApp->applicationName() + "/")),
          inputIdleTimer(this),
          backendTranslator(0),
          logFile(qApp->applicationName().append(".log")),
          targetsModel(0),
          pSelf(p)
    {
        // TODO: check install prefix
        skinPaths << "/usr/share/qtmediahub/skins/";
        skinPaths << QDir::homePath() + "/.qtmediahub/skins/";
        QDir skinPath(Config::value("skins", QString(basePath % "/skins")));
        if (skinPath.exists())
            skinPaths << skinPath.absolutePath();

        pluginPath = QDir(Config::value("plugins", QString(basePath % "/plugins"))).absolutePath();
        resourcePath = QDir(Config::value("resources", QString(basePath % "/resources"))).absolutePath();

        inputIdleTimer.setInterval(Config::value("idle-timeout", 120)*1000);
        inputIdleTimer.setSingleShot(true);
        inputIdleTimer.start();

        connect(&inputIdleTimer, SIGNAL(timeout()), pSelf, SIGNAL(inputIdle()));

        logFile.open(QIODevice::Text|QIODevice::ReadWrite);
        log.setDevice(&logFile);

        connect(&resourcePathMonitor,
                SIGNAL(directoryChanged(const QString &)),
                this,
                SLOT(handleDirChanged(const QString &)));

        foreach (QString skinPath, skinPaths)
            resourcePathMonitor.addPath(skinPath);
        resourcePathMonitor.addPath(pluginPath);

        QFileInfo thumbnailFolderInfo(thumbnailPath);
        if (!thumbnailFolderInfo.exists()) {
            QDir dir;
            dir.mkpath(thumbnailFolderInfo.absoluteFilePath());
        }

        discoverSkins();
    }

    ~BackendPrivate()
    {
        delete backendTranslator;
        backendTranslator = 0;
        qDeleteAll(pluginTranslators.begin(), pluginTranslators.end());
    }

public slots:
    void handleDirChanged(const QString &dir);

public:
    void resetLanguage();
    void discoverSkins();
    void discoverEngines();

    QSet<QString> advertizedEngineRoles;

    QList<QMHPlugin*> advertizedEngines;
    QList<QMHPlugin*> allEngines;

    const QString platformOffset;

    const QString basePath;
    QString pluginPath;
    QString resourcePath;
    const QString thumbnailPath;

    QStringList skinPaths;
    QList<QObject *> skins;

    QTimer inputIdleTimer;
    QTranslator *backendTranslator;
    QList<QTranslator*> pluginTranslators;
    QFile logFile;
    QTextStream log;
    QFileSystemWatcher resourcePathMonitor;

    QAbstractItemModel *targetsModel;

#if defined(Q_WS_S60) || defined(Q_WS_MAEMO)
    QNetworkConfigurationManager mgr;
    QNetworkSession *session;
#endif

    Backend *pSelf;
};

void BackendPrivate::handleDirChanged(const QString &dir)
{
    if(dir == pluginPath) {
        qWarning() << "Changes in plugin path, probably about to eat your poodle";
        discoverEngines();
    } else if(skinPaths.contains(dir)) {
        qWarning() << "Changes in skin path, repopulating skins";
        discoverSkins();
    }
}

void BackendPrivate::resetLanguage()
{
    static QString baseTranslationPath(basePath % "/translations/");
    const QString language = Backend::instance()->language();
    delete backendTranslator;
    backendTranslator = new QTranslator(this);
    backendTranslator->load(baseTranslationPath % language % ".qm");
    qApp->installTranslator(backendTranslator);

    qDeleteAll(pluginTranslators.begin(), pluginTranslators.end());

    foreach(QMHPlugin *plugin, allEngines) {
        QTranslator *pluginTranslator = new QTranslator(this);
        pluginTranslator->load(baseTranslationPath % plugin->role() % "_" % language % ".qm");
        pluginTranslators << pluginTranslator;
        qApp->installTranslator(pluginTranslator);
    }
}

void BackendPrivate::discoverSkins()
{
    skins.clear();

    foreach (QString skinPath, skinPaths) {
        QStringList potentialSkins = QDir(skinPath).entryList(QDir::Dirs | QDir::NoDotAndDotDot);

        foreach(const QString &currentPath, potentialSkins) {
            if(QFile(skinPath % "/" % currentPath % "/" % currentPath).exists()) {
                Skin *skin = new Skin(currentPath, skinPath % "/" % currentPath % "/");
                skins << skin;
            }
        }
    }

    qWarning() << "Available skins:";
    foreach(QObject *skin, skins)
        qWarning() << "\t" << qobject_cast<Skin*>(skin)->name();
}

void BackendPrivate::discoverEngines()
{
    foreach(const QString fileName, QDir(pluginPath).entryList(QDir::Files)) {
        QString qualifiedFileName(pluginPath % "/" % fileName);
        QPluginLoader pluginLoader(qualifiedFileName);

        if (!pluginLoader.load()) {
            qWarning() << tr("Cant load plugin: %1 returns %2").arg(qualifiedFileName).arg(pluginLoader.errorString());
            continue;
        }

        QMHPlugin *plugin = qobject_cast<QMHPlugin*>(pluginLoader.instance());
        if (!plugin)
            qWarning() << tr("Invalid QMH plugin present: %1").arg(qualifiedFileName);
        else if (!plugin->dependenciesSatisfied())
            qWarning() << tr("Can't meet dependencies for %1").arg(qualifiedFileName);
        else if (plugin->role() == "undefined")
            qWarning() << tr("Plugin %1 has an undefined role").arg(qualifiedFileName);
        else {
            plugin->setParent(this);
            allEngines << plugin;
            Backend::instance()->advertizeEngine(plugin);
        }
    }
    resetLanguage();
}

Backend::Backend(QObject *parent)
    : QObject(parent),
      d(new BackendPrivate(this))
{
#if defined(Q_WS_S60) || defined(Q_WS_MAEMO)
    // Set Internet Access Point
    QList<QNetworkConfiguration> activeConfigs = d->mgr.allConfigurations();
    if (activeConfigs.count() <= 0)
        return;

    QNetworkConfiguration cfg = activeConfigs.at(0);
    foreach(QNetworkConfiguration config, activeConfigs) {
        if (config.type() == QNetworkConfiguration::UserChoice) {
            cfg = config;
            break;
        }
    }

    session = new QNetworkSession(cfg);
    session->open();
    if (!session->waitForOpened(-1))
        return;
#endif

    QString dejavuPath(d->resourcePath % "/3rdparty/dejavu-fonts-ttf-2.32/ttf/");
    if (QDir(dejavuPath).exists()) {
        QFontDatabase::addApplicationFont(dejavuPath % "DejaVuSans.ttf");
        QFontDatabase::addApplicationFont(dejavuPath % "DejaVuSans-Bold.ttf");
        QFontDatabase::addApplicationFont(dejavuPath % "DejaVuSans-Oblique.ttf");
        QFontDatabase::addApplicationFont(dejavuPath % "DejaVuSans-BoldOblique.ttf");
        QApplication::setFont(QFont("DejaVu Sans"));
    }
}

Backend::~Backend()
{
#if defined(Q_WS_S60) || defined(Q_WS_MAEMO)
    session->close();
#endif

    delete d;
    d = 0;
}

void Backend::initialize()
{
    d->discoverEngines();
}

QString Backend::language() const
{
    //FIXME: derive from locale
    //Allow override
    return QString();
    //Bob is a testing translation placeholder
    //return QString("bob");
}

QList<QMHPlugin *> Backend::allEngines() const
{
    return d->allEngines;
}

QList<QObject *> Backend::advertizedEngines() const
{
    QList<QObject *> ret;
    foreach(QMHPlugin *engine, d->advertizedEngines) {
        ret << engine;
    }

    return ret;
}

QList<QObject *> Backend::skins() const
{
    return d->skins;
}

Backend* Backend::instance()
{
    if (!pSelf) {
        pSelf = new Backend();
    }
    return pSelf;
}

void Backend::destroy()
{
    delete pSelf;
    pSelf = 0;
}

QString Backend::basePath() const
{
    return d->basePath;
}

QString Backend::pluginPath() const
{
    return d->pluginPath;
}

QString Backend::resourcePath() const
{
    return d->resourcePath;
}

QString Backend::thumbnailPath() const
{
    return d->thumbnailPath;
}

bool Backend::transforms() const
{
#ifdef GL
    return (QGLFormat::hasOpenGL() && Config::isEnabled("transforms", true));
#else
    return false;
#endif
}

void Backend::advertizeEngine(QMHPlugin *engine)
{
    //Advertize to main menu
    QString role = engine->role();
    if (role.isEmpty())
        return;
    if (d->advertizedEngineRoles.contains(role)) {
        qWarning() << tr("Duplicate engine found for role %1").arg(role);
        return;
    } else {
        d->advertizedEngineRoles << role;
    }

    if (engine->advertized())
        d->advertizedEngines << engine;

    connect(engine, SIGNAL(pluginChanged()), this, SIGNAL(advertizedEnginesChanged()));
    emit advertizedEnginesChanged();
}

void Backend::openUrlExternally(const QUrl & url) const
{
    QDesktopServices::openUrl(url);
}

void Backend::log(const QString &logMsg) 
{
    qDebug() << logMsg;
    d->log << logMsg << endl;
}

bool Backend::eventFilter(QObject *obj, QEvent *event) {
    if (event->type() == QEvent::KeyPress
            || event->type() == QEvent::KeyRelease
            || event->type() == QEvent::MouseMove
            || event->type() == QEvent::MouseButtonPress)
    {
        d->inputIdleTimer.start();
    }

    return QObject::eventFilter(obj, event);
}

QObject *Backend::engineByRole(const QString &role) 
{
    foreach (QObject *currentEngine, d->advertizedEngines)
        if (qobject_cast<QMHPlugin*>(currentEngine)->role() == role)
            return currentEngine;
    qWarning() << tr("Seeking a non-existant plugin, prepare to die");
    return 0;
}

QObject *Backend::engineByName(const QString &name)
{
    foreach (QObject *currentEngine, d->allEngines)
        if (qobject_cast<QMHPlugin*>(currentEngine)->name() == name)
            return currentEngine;

    return 0;
}

QObject *Backend::targetsModel() const
{
    if (!d->targetsModel) {
#ifdef QMH_AVAHI
        QAvahiServiceBrowserModel *model = new QAvahiServiceBrowserModel(const_cast<Backend *>(this));
        model->setAutoResolve(true);
        model->browse("_qmh._tcp", QAvahiServiceBrowserModel::HideIPv6 | QAvahiServiceBrowserModel::HideLocal);
        d->targetsModel = model;
#else
        d->targetsModel = new QStandardItemModel(const_cast<Backend *>(this));
#endif
    }
    return d->targetsModel;
}

#include "backend.moc"
