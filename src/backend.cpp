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
#include "plugins/qmhplugininterface.h"
#include "plugins/qmhplugin.h"
#include "dataproviders/proxymodel.h"
#include "dataproviders/dirmodel.h"
#include "qmh-config.h"
#include "qml-extensions/qmlfilewrapper.h"
#include "qml-extensions/actionmapper.h"
#include "dataproviders/playlist.h"
#include "rpc/rpcconnection.h"

#ifndef QMH_NO_AVAHI
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
          skinPath(basePath % "/skins"),
          pluginPath(basePath % "/plugins"),
          resourcePath(basePath % "/resources"),
          // Use "large" instead of appName to fit freedesktop spec
          thumbnailPath(Config::value("thumbnail-path", QDir::homePath() + "/.thumbnails/" + qApp->applicationName() + "/")),
          inputIdleTimer(this),
          qmlEngine(0),
          backendTranslator(0),
          logFile(qApp->applicationName().append(".log")),
          targetsModel(0),
          pSelf(p)
    {
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

    QList<QObject*> advertizedEngines;

    const QString platformOffset;

    const QString basePath;
    const QString skinPath;
    const QString pluginPath;
    const QString resourcePath;
    const QString thumbnailPath;

    QTimer inputIdleTimer;
    QDeclarativeEngine *qmlEngine;
    QTranslator *backendTranslator;
    QList<QTranslator*> pluginTranslators;
    QFile logFile;
    QTextStream log;
    QFileSystemWatcher resourcePathMonitor;
    QStringList skins;

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
    } else if(dir == skinPath) {
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

    foreach(QObject *pluginObject, advertizedEngines) {
        QMHPlugin *plugin = qobject_cast<QMHPlugin*>(pluginObject);
        QTranslator *pluginTranslator = new QTranslator(this);
        pluginTranslator->load(baseTranslationPath % plugin->role() % "_" % language % ".qm");
        pluginTranslators << pluginTranslator;
        qApp->installTranslator(pluginTranslator);
    }
}

void BackendPrivate::discoverSkins()
{
    skins.clear();

    QStringList potentialSkins = QDir(skinPath).entryList(QDir::Dirs | QDir::NoDotAndDotDot);

    foreach(const QString &currentPath, potentialSkins)
        if(QFile(skinPath % "/" % currentPath % "/" % currentPath).exists())
            skins << currentPath;

    qWarning() << "Available skins" << skins;
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

        QMHPluginInterface* pluginInterface = qobject_cast<QMHPluginInterface*>(pluginLoader.instance());
        if (!pluginInterface)
            qWarning() << tr("Invalid QMH plugin present: %1").arg(qualifiedFileName);
        else if (!pluginInterface->dependenciesSatisfied())
            qWarning() << tr("Can't meet dependencies for %1").arg(qualifiedFileName);
        else if (pluginInterface->role() == "undefined")
            qWarning() << tr("Plugin %1 has an undefined role").arg(qualifiedFileName);
        else {
            QMHPlugin *plugin = new QMHPlugin(qobject_cast<QMHPluginInterface*>(pluginLoader.instance()), this);
            plugin->setParent(this);
            if(qmlEngine)
                plugin->registerPlugin(qmlEngine->rootContext());
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

    QFontDatabase::addApplicationFont(d->resourcePath + "/3rdparty/dejavu-fonts-ttf-2.32/ttf/DejaVuSans.ttf");
    QFontDatabase::addApplicationFont(d->resourcePath + "/3rdparty/dejavu-fonts-ttf-2.32/ttf/DejaVuSans-Bold.ttf");
    QFontDatabase::addApplicationFont(d->resourcePath + "/3rdparty/dejavu-fonts-ttf-2.32/ttf/DejaVuSans-Oblique.ttf");
    QFontDatabase::addApplicationFont(d->resourcePath + "/3rdparty/dejavu-fonts-ttf-2.32/ttf/DejaVuSans-BoldOblique.ttf");
    QApplication::setFont(QFont("DejaVu Sans"));
}

Backend::~Backend()
{
#if defined(Q_WS_S60) || defined(Q_WS_MAEMO)
    session->close();
#endif

    delete d;
    d = 0;
}

void Backend::initialize(QDeclarativeEngine *qmlEngine)
{
    // register dataproviders to QML
    qmlRegisterType<ActionMapper>("ActionMapper", 1, 0, "ActionMapper");
    qmlRegisterType<QMHPlugin>("QMHPlugin", 1, 0, "QMHPlugin");
    qmlRegisterType<ProxyModel>("ProxyModel", 1, 0, "ProxyModel");
    qmlRegisterType<ProxyModelItem>("ProxyModel", 1, 0, "ProxyModelItem");
    qmlRegisterType<DirModel>("DirModel", 1, 0, "DirModel");
    qmlRegisterType<QMLFileWrapper>("QMLFileWrapper", 1, 0, "QMLFileWrapper");
    qmlRegisterType<Playlist>("Playlist", 1, 0, "Playlist");
    qmlRegisterType<RpcConnection>("RpcConnection", 1, 0, "RpcConnection");

    if (qmlEngine) {
        //FIXME: We are clearly failing to keep the backend Declarative free :p
        d->qmlEngine = qmlEngine;
        qmlEngine->rootContext()->setContextProperty("backend", this);
        qmlEngine->rootContext()->setContextProperty("playlist", new Playlist);
    }

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

QList<QObject*> Backend::engines() const
{
    return d->advertizedEngines;
}

QStringList Backend::skins() const
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

QString Backend::skinPath() const 
{
    return d->skinPath;
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
    QString role = engine->property("role").toString();
    if (role.isEmpty())
        return;
    if (d->advertizedEngineRoles.contains(role)) {
        qWarning() << tr("Duplicate engine found for role %1").arg(role);
        return;
    }
    d->advertizedEngines << engine;
    if (d->qmlEngine)
        d->qmlEngine->rootContext()->setContextProperty(role % "Engine", engine);
    d->advertizedEngineRoles << role;
    emit enginesChanged();
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

// again dependent on declarative, needs to be fixed
void Backend::clearComponentCache() 
{
    if (d->qmlEngine) {
        d->qmlEngine->clearComponentCache();
    }
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

QObject* Backend::engine(const QString &role) 
{
    foreach (QObject *currentEngine, d->advertizedEngines)
        if (qobject_cast<QMHPlugin*>(currentEngine)->role() == role)
            return currentEngine;
    qWarning() << tr("Seeking a non-existant plugin, prepare to die");
    return 0;
}

QObject *Backend::targetsModel() const
{
    if (!d->targetsModel) {
#ifndef QMH_NO_AVAHI
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
