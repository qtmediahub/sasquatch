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
#include "frontend.h"

#include "qmh-config.h"
#include "rpc/rpcconnection.h"
#include "skin.h"
#include "scopedtransaction.h"
#include "media/mediaplugin.h"
#include "media/mediascanner.h"
#include "media/mediamodel.h"
#include "media/mediaparser.h"

#ifdef QMH_AVAHI
#include "qavahiservicebrowsermodel.h"
#else
#include "staticservicebrowsermodel.h"
#endif

#ifdef QT_SINGLE_APPLICATION
#include "qtsingleapplication.h"
#endif

#include <QDir>
#include <QString>
#include <QPluginLoader>
#include <QCoreApplication>
#include <QVariant>
#include <QFileSystemModel>
#include <QDesktopServices>
#include <QFileSystemWatcher>
#include <QNetworkProxy>

#include <QHostInfo>

#ifdef QMH_AVAHI
#include "qavahiservicepublisher.h"
#endif

#include <QDebug>

#define DATABASE_CONNECTION_NAME "Backend"

class SkinSelector : public QDialog
{
    Q_OBJECT
public:
    SkinSelector(QWidget *p = 0)
        : QDialog(p)
    {
        setAttribute(Qt::WA_DeleteOnClose);
        setModal(true);
        QVBoxLayout *vbox = new QVBoxLayout(this);
        QListWidget *skinsView = new QListWidget(this);

        connect(skinsView,
                SIGNAL(itemDoubleClicked(QListWidgetItem*)),
                this,
                SLOT(handleSkinSelection(QListWidgetItem*)));

        foreach(Skin *skin, Backend::instance()->skins())
            skinsView->addItem(skin->name());

        vbox->addWidget(skinsView);
    }

public slots:
    void handleSkinSelection(QListWidgetItem* item) {
        Backend::instance()->frontend()->setSkin(item->text());
        close();
    }
};

Backend* Backend::s_instance = 0;

class BackendPrivate : public QObject
{
    Q_OBJECT
public:
    BackendPrivate(Backend *p);
    ~BackendPrivate();

public slots:
    void handleDirChanged(const QString &dir);
    void selectSkin();

public:
    void resetLanguage();
    void discoverSkins();
    void discoverActions();
    void initializeMedia();

    bool primarySession;
    Frontend *frontend;

    QHash<QString, MediaPlugin *> engines;
    QList<QAction*> actions;

    const QString platformOffset;

    QString basePath;
    QString pluginPath;
    QString resourcePath;
    const QString thumbnailPath;

    QStringList skinPaths;
    QList<Skin *> skins;

    QTimer inputIdleTimer;
    QTranslator *backendTranslator;
    QList<QTranslator*> pluginTranslators;
    QFile logFile;
    QTextStream log;
    QFileSystemWatcher pathMonitor;

    QSystemTrayIcon *systray;
    QAbstractItemModel *targetsModel;

#if defined(Q_WS_S60) || defined(Q_WS_MAEMO)
    QNetworkConfigurationManager mgr;
    QNetworkSession *session;
#endif

    QSqlDatabase mediaDb;
    MediaScanner *mediaScanner;
    QThread *scannerThread;

    QAction *selectSkinAction;
    Backend *q;
};

BackendPrivate::BackendPrivate(Backend *p)
    : QObject(p),
      primarySession(true),
      frontend(0),
  #ifdef Q_OS_MAC
      platformOffset("/../../.."),
  #endif
      // Use "large" instead of appName to fit freedesktop spec
      thumbnailPath(Config::value("thumbnail-path", QDir::homePath() + "/.thumbnails/" + qApp->applicationName() + "/")),
      inputIdleTimer(this),
      backendTranslator(0),
      logFile(Backend::storageLocation(QDesktopServices::TempLocation)
          .append("/")
          .append(qApp->applicationName())
          .append(".log")),
      systray(0),
      targetsModel(0),
      mediaScanner(0),
      scannerThread(0),
      q(p)
{
    QString defaultBasePath("/usr/share/qtmediahub/");
    if (Config::value("testing", false) || !QDir(defaultBasePath).exists()) {
        qDebug() << "Either testing or uninstalled: running in build dir";
        defaultBasePath = QCoreApplication::applicationDirPath() + platformOffset;
    }
    basePath = Config::value("base-path",  defaultBasePath);

    QNetworkProxy proxy;
    if (Config::isEnabled("proxy", false)) {
        QString proxyHost(Config::value("proxy-host", "localhost").toString());
        int proxyPort = Config::value("proxy-port", 8080);
        proxy.setType(QNetworkProxy::HttpProxy);
        proxy.setHostName(proxyHost);
        proxy.setPort(proxyPort);
        QNetworkProxy::setApplicationProxy(proxy);
        qWarning() << "Using proxy host"
            << proxyHost
            << "on port"
            << proxyPort;
    }

    selectSkinAction = new QAction(tr("Select skin"), this);
    QAction *quitAction = new QAction(tr("Quit"), this);
    connect(selectSkinAction, SIGNAL(triggered()), this, SLOT(selectSkin()));
    connect(quitAction, SIGNAL(triggered()), qApp, SLOT(quit()));
    actions.append(selectSkinAction);
    actions.append(quitAction);

    // TODO: check install prefix
    skinPaths << QCoreApplication::applicationDirPath() + "/../../skins/"; // unified repo
    skinPaths << "/usr/share/qtmediahub/skins/";
    skinPaths << QDir::homePath() + "/.qtmediahub/skins/";
    skinPaths << QDir(Config::value("skins-path", QString(basePath % "/skins"))).absolutePath();
    if (!qgetenv("QMH_SKINPATH").isEmpty())
        skinPaths << QDir(qgetenv("QMH_SKINPATH")).absolutePath();

    pluginPath = QDir(Config::value("plugins-path", QString(basePath % "/plugins"))).absolutePath();

    if (!qgetenv("QMH_RESOURCEPATH").isEmpty())
        resourcePath = QDir(qgetenv("QMH_RESOURCEPATH")).absolutePath();
    else
        resourcePath = QDir(Config::value("resources-path", QString(basePath % "/resources"))).absolutePath();

    inputIdleTimer.setInterval(Config::value("idle-timeout", 120)*1000);
    inputIdleTimer.setSingleShot(true);
    inputIdleTimer.start();

    connect(&inputIdleTimer, SIGNAL(timeout()), q, SIGNAL(inputIdle()));

    logFile.open(QIODevice::Text|QIODevice::ReadWrite);
    log.setDevice(&logFile);

    connect(&pathMonitor, SIGNAL(directoryChanged(QString)), this, SLOT(handleDirChanged(QString)));

    foreach (QString skinPath, skinPaths) {
        if (QDir(skinPath).exists())
            pathMonitor.addPath(skinPath);
    }
    pathMonitor.addPath(pluginPath);

    QFileInfo thumbnailFolderInfo(thumbnailPath);
    if (!thumbnailFolderInfo.exists()) {
        QDir dir;
        dir.mkpath(thumbnailFolderInfo.absoluteFilePath());
    }
    discoverSkins();

    initializeMedia();
}

BackendPrivate::~BackendPrivate()
{
    delete frontend;
    frontend = 0;

    delete backendTranslator;
    backendTranslator = 0;

    //This clean up is arguably a waste of effort since
    //the death of the backend marks the death of the appliction
    qDeleteAll(pluginTranslators.begin(), pluginTranslators.end());
    qDeleteAll(engines.begin(), engines.end());
    qDeleteAll(skins.begin(), skins.end());
    qDeleteAll(actions.begin(), actions.end());

    delete backendTranslator;
    delete systray;
    delete targetsModel;

#if defined(Q_WS_S60) || defined(Q_WS_MAEMO)
    delete session;
#endif

    mediaScanner->stop();
    mediaScanner->deleteLater();
    scannerThread->quit();
    scannerThread->wait();

    mediaDb = QSqlDatabase();
    QSqlDatabase::removeDatabase(DATABASE_CONNECTION_NAME);
}

void BackendPrivate::handleDirChanged(const QString &dir)
{
    if(dir == pluginPath) {
        qWarning() << "Changes in plugin path, probably about to eat your poodle";
    } else if(skinPaths.contains(dir)) {
        qWarning() << "Changes in skin path, repopulating skins";
        discoverSkins();
    }
}

void BackendPrivate::selectSkin()
{
    SkinSelector *skinSelector = new SkinSelector;
    skinSelector->show();
}

void BackendPrivate::resetLanguage()
{
    static QString baseTranslationPath(basePath % "/translations/");
    const QString language = q->language();
    delete backendTranslator;
    backendTranslator = new QTranslator(this);
    backendTranslator->load(baseTranslationPath % language % ".qm");
    qApp->installTranslator(backendTranslator);

    qDeleteAll(pluginTranslators.begin(), pluginTranslators.end());
}

void BackendPrivate::discoverSkins()
{
    qDeleteAll(skins);
    skins.clear();

    foreach (QString skinPath, skinPaths) {
        QStringList potentialSkins = QDir(skinPath).entryList(QDir::Dirs | QDir::NoDotAndDotDot);

        foreach(const QString &currentPath, potentialSkins) {
            const QString prospectiveFilename = skinPath % "/" % currentPath % "/" % "skin.map";
            if(QFile(prospectiveFilename).exists()) {
                Skin *skin = new Skin(prospectiveFilename, this);
                skins << skin;
            }
        }
    }

    if (skins.isEmpty()) {
        qWarning() << "No skins are found in your skin paths"<< endl \
                   << "If you don't intend to run this without skins"<< endl \
                   << "Please read the INSTALL doc available here:" \
                   << "http://gitorious.org/qtmediahub/qtmediahub-core/blobs/master/INSTALL" \
                   << "for further details";
    } else {
        QStringList sl;
        foreach(Skin *skin, skins)
            sl.append(skin->name());
        qDebug() << "Available skins:" << sl.join(",");
    }
}

void Backend::loadEngines()
{
    QStringList loaded;
    foreach(const QString &fileName, QDir(d->pluginPath).entryList(QDir::Files)) {
        QString absoluteFilePath(d->pluginPath % "/" % fileName);
        QPluginLoader pluginLoader(absoluteFilePath);

        if (!pluginLoader.load()) {
            qWarning() << tr("Cant load plugin: %1 returns %2").arg(absoluteFilePath).arg(pluginLoader.errorString());
            continue;
        }

        MediaPlugin *plugin = qobject_cast<MediaPlugin*>(pluginLoader.instance());
        if (!plugin) {
            qWarning() << tr("Invalid media plugin present: %1").arg(absoluteFilePath);
            pluginLoader.unload();
            continue;
        }
        foreach(const QString &key, plugin->parserKeys()) {
            MediaParser *parser = plugin->createParser(key);
            plugin->setParent(this);
            QMetaObject::invokeMethod(d->mediaScanner, "addParser", Qt::QueuedConnection, 
                                      Q_ARG(MediaParser *, parser));
        }
    }
}

void BackendPrivate::discoverActions()
{
    selectSkinAction->setEnabled(!skins.isEmpty() && frontend);
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

    QMetaObject::invokeMethod(this, "initialize", Qt::QueuedConnection);
}

Backend::~Backend()
{
#if defined(Q_WS_S60) || defined(Q_WS_MAEMO)
    session->close();
#endif

    delete d;
    d = 0;

    s_instance = 0;
}

QString Backend::storageLocation(QDesktopServices::StandardLocation type)
{
    QString location = QDesktopServices::storageLocation(type);
    return location.isEmpty() ? QString("/tmp") : location;
}

void Backend::initialize()
{
    if (!Config::isEnabled("headless", false)) {
        d->frontend = new Frontend();
    }
    if (Config::isEnabled("systray", true)) {
        d->systray = new QSystemTrayIcon(QIcon(":/images/petite-ganesh-22x22.jpg"), this);
        d->systray->setVisible(true);
        QMenu *contextMenu = new QMenu;
        foreach(QAction *action, d->actions)
            contextMenu->addAction(action);

        d->systray->setContextMenu(contextMenu);
    }

#ifdef QMH_AVAHI
    if (d->primarySession && Config::isEnabled("avahi", true) && Config::isEnabled("avahi-advertize", true)) {
        QAvahiServicePublisher *publisher = new QAvahiServicePublisher(this);
        publisher->publish(QHostInfo::localHostName(), "_qtmediahub._tcp", 1234, "Qt Media Hub JSON-RPCv2 interface");
        qDebug() << "Advertizing session via zeroconf";
    } else {
        qDebug() << "Failing to advertize session via zeroconf";
    }
#endif

    // This is here because MediaScanner::initialize() uses Backend::instance()
    QMetaObject::invokeMethod(d->mediaScanner, "initialize", Qt::QueuedConnection);
}

void BackendPrivate::initializeMedia()
{
    if (!QSqlDatabase::isDriverAvailable("QSQLITE")) {
        qFatal("The SQLITE driver is unavailable");
        return;
    }

    const QString DATABASE_NAME(Backend::storageLocation(QDesktopServices::DataLocation).append("/media.db"));
    QDir dir;
    dir.mkpath(Backend::storageLocation(QDesktopServices::DataLocation));
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", DATABASE_CONNECTION_NAME);
    db.setDatabaseName(DATABASE_NAME);

    if (!db.open()) {
        qFatal("Failed to open SQLITE database %s", qPrintable(db.lastError().text()));
        return;
    }

    mediaDb = db;

    if (db.tables().isEmpty()) {
        ScopedTransaction transaction(db);
        transaction.execFile(":/media/schema.sql");
    }

    scannerThread = new QThread(q);
    mediaScanner = new MediaScanner;
    mediaScanner->moveToThread(scannerThread);
    scannerThread->start();
}

MediaScanner *Backend::mediaScanner() const
{
    return d->mediaScanner;
}

QSqlDatabase Backend::mediaDatabase() const
{
    return d->mediaDb;
}

QString Backend::language() const
{
    //FIXME: derive from locale
    //Allow override
    return QString();
    //Bob is a testing translation placeholder
    //return QString("bob");
}

QList<Skin*> Backend::skins() const
{
    return d->skins;
}

QList<QAction*> Backend::actions() const
{
    return d->actions;
}

Backend* Backend::instance()
{
    if (!s_instance) {
        s_instance = new Backend();
    }
    return s_instance;
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

Frontend* Backend::frontend() const
{
    return d->frontend;
}


QObject *Backend::targetsModel() const
{
    if (!d->targetsModel) {
#ifdef QMH_AVAHI
        if (Config::isEnabled("avahi", true)) {
            QAvahiServiceBrowserModel *model = new QAvahiServiceBrowserModel(const_cast<Backend *>(this));
            model->setAutoResolve(true);
            QAvahiServiceBrowserModel::Options options = QAvahiServiceBrowserModel::NoOptions;
            if (Config::isEnabled("avahi-hide-ipv6"), true)
                options |= QAvahiServiceBrowserModel::HideIPv6;
            if (Config::isEnabled("avahi-hide-local", true))
                options |= QAvahiServiceBrowserModel::HideLocal;
            model->browse("_qtmediahub._tcp", options);
            d->targetsModel = model;
        }
#else
        d->targetsModel = new StaticServiceBrowserModel(const_cast<Backend *>(this));
#endif
    }
    return d->targetsModel;
}

void Backend::addMediaSearchPath(const QString &type, const QString &name, const QString &path)
{
    MediaModel *model = new MediaModel(this);
    model->setMediaType(type);
    connect(model, SIGNAL(scanFinished()), model, SLOT(deleteLater()));
    model->addSearchPath(path, name);
}

void Backend::removeMediaSearchPath(const QString &type, const QString &name)
{
    MediaModel *model = new MediaModel(this);
    model->setMediaType(type);
    model->removeSearchPath(name);
    model->deleteLater();
}

QStringList Backend::findApplications() const
{
    static QByteArray env = qgetenv("QMH_APPPATH");

    QStringList appSearchPaths;
    if (!env.isEmpty())
        appSearchPaths << env;
    appSearchPaths << QCoreApplication::applicationDirPath() + "/../../apps/"; // unified repo

    QStringList apps;

    foreach(const QString &appSearchPath, appSearchPaths) {
        QStringList subdirs = QDir(appSearchPath).entryList(QDir::AllDirs | QDir::NoDotAndDotDot);
        foreach(const QString &subdir, subdirs)  {
            QString appPath(appSearchPath + '/' + subdir + '/');
            QString fileName(appPath + "qmhmanifest.qml"); // look for apps/x/qmhmanifest.qml
            QFile prospectiveFile(fileName);
            if (prospectiveFile.exists())
                apps << (QDir(appPath).absolutePath() + '/');
        }
    }
    return apps;
}

void Backend::setPrimarySession(bool primarySession)
{
    d->primarySession = primarySession;
}


#include "backend.moc"
