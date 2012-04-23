/** This file is part of Qt Media Hub**

Copyright (c) 2012 Nokia Corporation and/or its subsidiary(-ies).*
All rights reserved.

Contact:  Nokia Corporation qmh-development@qt-project.org

You may use this file under the terms of the BSD license
as follows:

Redistribution and use in source and binary forms, with or
without modification, are permitted provided that the following
conditions are met:
* Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.

* Neither the name of Nokia Corporation and its Subsidiary(-ies)
nor the names of its contributors may be used to endorse or promote
products derived from this software without specific prior
written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. **/

#include "metrics.h"

#include "skinmanager.h"
#include "skinruntime.h"
#include "mediaserver.h"

#include <QtGui>
#include <QDebug>

#ifdef QMH_STATIC_SERVICE_BROWSER
#include "staticservicebrowsermodel.h"
#else
#include "simpleservicebrowsermodel.h"
#endif

#include "declarativeview.h"

#ifndef QT5
#include <QtDeclarative>
#endif

#ifndef NO_DBUS
#include <QDBusError>
#include <QDBusConnection>
#endif

#if defined(GL) && !defined(QT5)
#include <QGLFormat>
#endif

#if defined(GL) && !defined(QT5)
#include <QGLWidget>
#endif

#include "dirmodel.h"

#include "mainwindow.h"
#include "media/playlist.h"
#include "file.h"
#include "media/mediamodel.h"
#include "media/mediascanner.h"
#include "actionmapper.h"
#include "rpc/rpcconnection.h"
#include "libraryinfo.h"
#include "qmldebugging.h"
#include "actionmapper.h"
#include "trackpad.h"
#include "processmanager.h"
#include "devicemanager.h"
#include "powermanager.h"
#include "rpc/mediaplayerrpc.h"
#include "abstractmediaplayer.h"
#include "globalsettings.h"
#include "appsmanager.h"
#include "pushqml.h"
#include "ipaddressfinder.h"
#include "deviceexposure.h"
#include "inputnotifier.h"

#ifdef MEDIAPLAYER_DBUS
#include "mediaplayerdbus.h"
#elif defined(MEDIAPLAYER_VLC)
#include "mediaplayervlc.h"
#elif defined(MEDIAPLAYER_7425)
#include "mediaplayer7425.h"
#elif defined(MEDIAPLAYER_ST7105)
#include "mediaplayerST7105.h"
#elif defined(MEDIAPLAYER_ST7108)
#include "mediaplayerST7108.h"
#elif defined(MEDIAPLAYER_ST7540)
#include "mediaplayerST7540.h"
#elif defined(MEDIAPLAYER_TRIDENT_SHINER_GSTTSPLAYER)
#include "mediaplayerTridentShinerGstTsPlayer.h"
#elif defined(MEDIAPLAYER_TRIDENT_SHINER_MINIPLAYER)
#include "mediaplayerTridentShinerMiniplayer.h"
#else
#include "mediaplayerdummy.h"
#endif

#include "customcursor.h"
#include "httpserver/httpserver.h"

#include "contextcontentrpc.h"

#ifndef NO_DBUS
static void registerObjectWithDbus(const QString &path, QObject *object)
{
    bool objectRegistration =
            QDBusConnection::sessionBus().registerObject(path, object,
                                                         QDBusConnection::ExportScriptableSlots|QDBusConnection::ExportScriptableSignals);
    if (!objectRegistration)
        qDebug() << "Can't seem to register object with dbus service:" << QDBusConnection::sessionBus().lastError().message();
}
#endif

class SkinRuntimePrivate : public QObject
{
    Q_OBJECT
public:
    SkinRuntimePrivate(GlobalSettings *s, SkinRuntime *p);
    ~SkinRuntimePrivate();

public slots:
    DeclarativeView *declarativeView();

    QObject *loadQmlSkin(const QUrl &url);
    QObject *loadSkinSelector();

    // TODO check if there is some better place
    void rpcSendInputMethodStart();
    void rpcSendInputMethodStop();
    void rpcSendNewContextContent(QString &skinName, QString &contentName, QList<int> idList);
    void rpcSendInvalidateContextContent();
    void initialStatusCheck();
    void deadmanStatusCheck();
    void handleWarnings(const QList<QDeclarativeError> &warnings);

public:
    void enableRemoteControlMode(bool enable);

    bool dbusRegistration;
    bool remoteControlMode;

    QString errorMsg;
    QObject *skinUI;

    MainWindow *mainWindow;
    MediaServer *mediaServer;
    ProcessManager *processManager;
    DeviceManager *deviceManager;
    PowerManager *powerManager;
    MediaPlayerRpc *mediaPlayerRpc;
    RpcConnection *rpcConnection;
    DeviceExposure *deviceExposure;

    ActionMapper *actionMapper;
    Trackpad *trackpad;
    Skin *currentSkin;
    QAbstractItemModel *remoteSessionsModel;
    InputNotifier *inputNotifier;
    GlobalSettings *settings;
    SkinRuntime *q;
};

SkinRuntimePrivate::SkinRuntimePrivate(GlobalSettings *s, SkinRuntime *p)
    : QObject(p),
      dbusRegistration(false),
      remoteControlMode(true),
      skinUI(0),
      mediaServer(0),
      processManager(0),
      deviceManager(0),
      powerManager(0),
      mediaPlayerRpc(0),
      rpcConnection(0),
      trackpad(0),
      remoteSessionsModel(0),
      inputNotifier(0),
      settings(s),
      q(p)
{
#ifndef NO_DBUS
    dbusRegistration = QDBusConnection::sessionBus().registerService(QMH_DBUS_SERVICENAME);
    if (!dbusRegistration) {
        qDebug() << "Can't seem to register dbus service:" << QDBusConnection::sessionBus().lastError().message();
    }
#endif

#if defined(GL) && !defined(QT5)
    //Can't fool with the default format when dealing with the Tegra for some reason
    if (settings->isEnabled(GlobalSettings::OpenGLUsage) && settings->isEnabled(GlobalSettings::OpenGLFormatHack)) {
        QGLFormat format = QGLFormat::defaultFormat();
        //explicitly set options
        //format.setDoubleBuffer(true);
        //Doc: Text antialiasing in Qt 4 OpenGL engine?
        format.setSampleBuffers(true);
        //format.setAlpha(true);
        //Doc: Will screw with Quick 3D?
        //format.setDepth(false);
        //Doc: To what extent does this work prior to Qt 5?
        //format.setSwapInterval(1);
        QGLFormat::setDefaultFormat(format);
    }
#endif //GL

    foreach (const QString &resourcePath, LibraryInfo::resourcePaths(settings)) {
        QString dejavuPath(resourcePath % "/3rdparty/dejavu-fonts-ttf-2.32/ttf/");
        if (QDir(dejavuPath).exists()) {
            qDebug() << "Using the application specified dejavu font";
            QFontDatabase::addApplicationFont(dejavuPath % "DejaVuSans.ttf");
            QFontDatabase::addApplicationFont(dejavuPath % "DejaVuSans-Bold.ttf");
            QFontDatabase::addApplicationFont(dejavuPath % "DejaVuSans-Oblique.ttf");
            QFontDatabase::addApplicationFont(dejavuPath % "DejaVuSans-BoldOblique.ttf");
#ifdef QT5
            QGuiApplication::setFont(QFont("DejaVu Sans"));
#else
            QApplication::setFont(QFont("DejaVu Sans"));
#endif
            break;
        }
    }

    qApp->setOverrideCursor(Qt::BlankCursor);

    qRegisterMetaType<QModelIndex>();

    //Declarative is a hard dependency at present in any case
    // register dataproviders to QML
    qmlRegisterUncreatableType<ActionMapper>("ActionMapper", 1, 0, "ActionMapper", "For enums. For methods use actionmap global variable");
    qmlRegisterUncreatableType<AbstractMediaPlayer>("AbstractMediaPlayer", 1, 0, "AbstractMediaPlayer", "For enums. For methods use actionmap global variable");
    qmlRegisterType<Metrics>("Metrics", 1, 0, "Metrics");
    qmlRegisterType<DirModel>("DirModel", 1, 0, "DirModel");
    qmlRegisterType<Playlist>("Playlist", 1, 0, "Playlist");
    qmlRegisterType<MediaModel>("MediaModel", 1, 0, "MediaModel");
    qmlRegisterType<RpcConnection>("RpcConnection", 1, 0, "RpcConnection");
    qmlRegisterType<Settings>("Settings", 1, 0, "Settings");
    qmlRegisterType<PushQML>("PushQML", 1, 0, "PushQML");
    qmlRegisterType<IpAddressFinder>("IpAddressFinder", 1, 0, "IpAddressFinder");

    if (settings->isEnabled(GlobalSettings::OverlayMode)) {
#ifdef MEDIAPLAYER_DBUS
        qmlRegisterType<MediaPlayerDbus>("OverlayModeMediaPlayer", 1, 0, "OverlayModeMediaPlayer");
#elif defined(MEDIAPLAYER_VLC)
        qmlRegisterType<MediaPlayerVLC>("OverlayModeMediaPlayer", 1, 0, "OverlayModeMediaPlayer");
#elif defined(MEDIAPLAYER_7425)
        qmlRegisterType<MediaPlayer7425>("OverlayModeMediaPlayer", 1, 0, "OverlayModeMediaPlayer");
#elif defined(MEDIAPLAYER_ST7105)
        qmlRegisterType<MediaPlayerST7105>("OverlayModeMediaPlayer", 1, 0, "OverlayModeMediaPlayer");
#elif defined(MEDIAPLAYER_ST7108)
        qmlRegisterType<MediaPlayerST7108>("OverlayModeMediaPlayer", 1, 0, "OverlayModeMediaPlayer");
#elif defined(MEDIAPLAYER_ST7540)
        qmlRegisterType<MediaPlayerST7540>("OverlayModeMediaPlayer", 1, 0, "OverlayModeMediaPlayer");
#elif defined(MEDIAPLAYER_TRIDENT_SHINER_GSTTSPLAYER)
        qmlRegisterType<MediaPlayerTridentShinerGstTsPlayer>("OverlayModeMediaPlayer", 1, 0, "OverlayModeMediaPlayer");
#elif defined(MEDIAPLAYER_TRIDENT_SHINER_MINIPLAYER)
        qmlRegisterType<MediaPlayerTridentShinerMiniplayer>("OverlayModeMediaPlayer", 1, 0, "OverlayModeMediaPlayer");
#else
        qmlRegisterType<MediaPlayerDummy>("OverlayModeMediaPlayer", 1, 0, "OverlayModeMediaPlayer");
#endif
    }

#ifdef QMH_STATIC_SERVICE_BROWSER
    remoteSessionsModel = new StaticServiceBrowserModel(this);
#else
    remoteSessionsModel = new SimpleServiceBrowserModel(this);
#endif

    deviceExposure = new DeviceExposure(this);
}

SkinRuntimePrivate::~SkinRuntimePrivate()
{
    settings->setValue(GlobalSettings::Skin, currentSkin->name());
}

DeclarativeView *SkinRuntimePrivate::declarativeView()
{
    if (skinUI)
        skinUI->deleteLater();

    DeclarativeView *declarativeWidget = new DeclarativeView(settings);

    QDeclarativeEngine *engine = declarativeWidget->engine();
    QObject::connect(engine, SIGNAL(quit()), qApp, SLOT(quit()));

    skinUI = declarativeWidget;

    return declarativeWidget;
}

QObject *SkinRuntimePrivate::loadQmlSkin(const QUrl &targetUrl)
{
    DeclarativeView *declarativeWidget = declarativeView();

    QDeclarativeEngine *engine = declarativeWidget->engine();

    QDeclarativePropertyMap *runtime = new QDeclarativePropertyMap(declarativeWidget);
    if (!remoteControlMode) {
        runtime->insert("mediaScanner", qVariantFromValue(static_cast<QObject *>(mediaServer->mediaScanner())));
        runtime->insert("httpServer", qVariantFromValue(static_cast<QObject *>(mediaServer->httpServer())));
        runtime->insert("actionMapper", qVariantFromValue(static_cast<QObject *>(actionMapper)));
        runtime->insert("trackpad", qVariantFromValue(static_cast<QObject *>(trackpad)));
        runtime->insert("mediaPlayerRpc", qVariantFromValue(static_cast<QObject *>(mediaPlayerRpc)));
        runtime->insert("processManager", qVariantFromValue(static_cast<QObject *>(processManager)));
        runtime->insert("deviceManager", qVariantFromValue(static_cast<QObject *>(deviceManager)));
        runtime->insert("powerManager", qVariantFromValue(static_cast<QObject *>(powerManager)));

        actionMapper->setRecipient(declarativeWidget);
        trackpad->setRecipient(declarativeWidget);
    }
    runtime->insert("settings", qVariantFromValue(static_cast<QObject *>(settings)));
    runtime->insert("window", qVariantFromValue(static_cast<QObject *>(mainWindow)));
    runtime->insert("view", qVariantFromValue(static_cast<QObject *>(declarativeWidget)));
    runtime->insert("cursor", qVariantFromValue(static_cast<QObject *>(new CustomCursor(settings, runtime))));
    runtime->insert("skin", qVariantFromValue(static_cast<QObject *>(currentSkin)));
    runtime->insert("apps", qVariantFromValue(static_cast<QObject *>(new AppsManager(settings, this))));
    runtime->insert("file", qVariantFromValue(static_cast<QObject *>(new File(this))));
    runtime->insert("remoteSessionsModel", qVariantFromValue(static_cast<QObject *>(remoteSessionsModel)));

    ContextContentRpc* ccrpc = new ContextContentRpc(this);
    connect(ccrpc, SIGNAL(sendNewContextContent(QString&, QString&, QList<int>)), this, SLOT(rpcSendNewContextContent(QString&,QString&,QList<int>)));
    connect(ccrpc, SIGNAL(sendInvalidateContextContent()), this, SLOT(rpcSendInvalidateContextContent()));
    rpcConnection->registerObject(ccrpc);

    runtime->insert("contextContent", qVariantFromValue(static_cast<QObject*>(ccrpc)));

    declarativeWidget->rootContext()->setContextProperty("runtime", runtime);

    const QString mediaPlayer = settings->isEnabled(GlobalSettings::OverlayMode) ? "overlaymode" : "mobility";
    foreach (const QString &qmlImportPath, LibraryInfo::qmlImportPaths(settings)) {
        engine->addImportPath(qmlImportPath);
        engine->addImportPath(qmlImportPath % "/QtMediaHub/components/media/" % mediaPlayer); // ## is this correct?
    }
    engine->addImportPath(currentSkin->path());

    QObject::connect(engine, SIGNAL(warnings(QList<QDeclarativeError>)), this, SLOT(handleWarnings(QList<QDeclarativeError>)));

    //Would be nice to have error handling here for broken QML skins
    //Can't detect QML1/QDeclarativeView collision
    declarativeWidget->setSource(targetUrl);

    QTimer *deadmansTimer = new QTimer(declarativeWidget);
    //Give the device 2.5 seconds:
    //1) splash to complete
    //2) to get this parsed
    deadmansTimer->setInterval(2500);
    deadmansTimer->setSingleShot(true);
    QObject::connect(deadmansTimer, SIGNAL(timeout()), this, SLOT(deadmanStatusCheck()));
    deadmansTimer->start();

    QMetaObject::invokeMethod(this, "initialStatusCheck", Qt::QueuedConnection);

    return declarativeWidget;
}

void SkinRuntimePrivate::initialStatusCheck()
{
    DeclarativeView *declarativeWidget = qobject_cast<DeclarativeView*>(skinUI);
    if (declarativeWidget->status() != 1) {
        qDebug() << "Abandoning skin due to errors; time to fallback baby";
        mainWindow->setSkin(0);
    }
}

void SkinRuntimePrivate::deadmanStatusCheck()
{
    DeclarativeView *declarativeWidget = qobject_cast<DeclarativeView*>(skinUI);
    QVariant rationalSkin(declarativeWidget->rootObject()->property("rational"));
    if (!rationalSkin.isNull() && !rationalSkin.toBool()) {
        qDebug() << "Skin siezed; failing rationality test";
        mainWindow->setSkin(0);
    }
}

void SkinRuntimePrivate::handleWarnings(const QList<QDeclarativeError> &warnings)
{
    errorMsg.clear();
    foreach(const QDeclarativeError error, warnings) {
        errorMsg += error.toString() + "\n";
    }
}

QObject *SkinRuntimePrivate::loadSkinSelector()
{
    DeclarativeView *declarativeWidget = declarativeView();

    QDeclarativePropertyMap *runtime = new QDeclarativePropertyMap(declarativeWidget);
    runtime->insert("skinManager", qVariantFromValue(static_cast<QObject *>(new SkinManager(settings, declarativeWidget))));
    runtime->insert("window", qVariantFromValue(static_cast<QObject *>(mainWindow)));
    runtime->insert("skinruntime", qVariantFromValue(static_cast<QObject *>(q)));

    declarativeWidget->rootContext()->setContextProperty("runtime", runtime);

    declarativeWidget->setSource(QUrl("qrc:///skinselector.qml"));

    return declarativeWidget;
}

void SkinRuntimePrivate::enableRemoteControlMode(bool enable)
{
    if (remoteControlMode == enable)
        return;

    remoteControlMode = enable;

    if (enable) {
        delete processManager;
        processManager = 0;

        delete deviceManager;
        deviceManager = 0;
        delete powerManager;
        powerManager = 0;

        rpcConnection->unregisterObject(mediaPlayerRpc);
        delete mediaPlayerRpc;
        mediaPlayerRpc = 0;

        rpcConnection->unregisterObject(trackpad);
        delete trackpad;
        trackpad = 0;

        rpcConnection->unregisterObject(actionMapper);
        delete actionMapper;
        actionMapper = 0;

        delete rpcConnection;

        delete mediaServer;
        mediaServer = 0;

        return;
    }

    mediaServer = new MediaServer(settings, this);
    rpcConnection = new RpcConnection(RpcConnection::Server,
                                  #ifdef QT5
                                      QHostAddress::AnyIPv4,
                                  #else
                                      QHostAddress::Any,
                                  #endif
                                      1234,
                                      this);
    mediaPlayerRpc = new MediaPlayerRpc(this);
    mediaPlayerRpc->setObjectName("qmhmediaplayer");
    trackpad = new Trackpad(this);
    trackpad->setObjectName("trackpad");
    actionMapper = new ActionMapper(settings, this);
    actionMapper->setObjectName("qmhrpc");

    rpcConnection->registerObject(actionMapper);
    rpcConnection->registerObject(mediaPlayerRpc);
    rpcConnection->registerObject(trackpad);

    inputNotifier = new InputNotifier(this);
    connect(inputNotifier, SIGNAL(inputMethodStartRequested()), this, SLOT(rpcSendInputMethodStart()));
    connect(inputNotifier, SIGNAL(inputMethodStopRequested()), this, SLOT(rpcSendInputMethodStop()));

    processManager = new ProcessManager(this);

#ifndef NO_DBUS
//Segmentation fault on mac!
    if (QDBusConnection::systemBus().isConnected()) {
        deviceManager = new DeviceManager(this);
        powerManager = new PowerManager(this);
    }

    if (dbusRegistration) {
        ::registerObjectWithDbus("/mediacontrol", mediaPlayerRpc);
    }
#endif
}

void SkinRuntimePrivate::rpcSendInputMethodStart()
{
    if (!rpcConnection)
        return;

    rpcConnection->call("inputContext.inputMethodStartRequested");
}

void SkinRuntimePrivate::rpcSendInputMethodStop()
{
    if (!rpcConnection)
        return;

    rpcConnection->call("inputContext.inputMethodStopRequested");
}

void SkinRuntimePrivate::rpcSendNewContextContent(QString &skinName, QString &contentName, QList<int> idList)
{
    if (!rpcConnection)
        return;

    QVariantList vList;
    for(int i = 0; i< idList.length(); i++)
        vList.append(QVariant(idList[i]));

    rpcConnection->call("contextContent.newContextContent", skinName, contentName, vList);
}

void SkinRuntimePrivate::rpcSendInvalidateContextContent()
{
    if (!rpcConnection)
        return;

    rpcConnection->call("contextContent.invalidateContextContent");
}


SkinRuntime::SkinRuntime(GlobalSettings *settings, MainWindow *p)
    : QObject(p),
      d(new SkinRuntimePrivate(settings, this))
{
    d->mainWindow = p;
}

SkinRuntime::~SkinRuntime()
{
    delete d;
    d = 0;
}

QString SkinRuntime::errorMsg() const
{
    return d->errorMsg;
}

QObject *SkinRuntime::create(Skin *skin)
{
    const QSize res = d->settings->value(GlobalSettings::SkinResolution).toRect().size(); // TODO provide a toSize for Settings
    const QSize preferredResolution = res.isEmpty()
#ifdef QT5
        ? qApp->primaryScreen()->geometry().size()
#else
        ? qApp->desktop()->screenGeometry().size()
#endif
        : res;

    QObject *interface = 0;

    if (skin)
    {
        bool fallback = false;

        skin->parseManifest();

        QUrl url = skin->urlForResolution(preferredResolution);
        if (!url.isValid()) {
            qWarning() << "Malformed URL " << url;
            fallback = true;
        }

        if (skin->type(url) != Skin::Qml) {
            qWarning() << "Only QML skins supported at present: Error loading skin " << skin->name();
            fallback = true;
        }

        if (!fallback) {
            d->currentSkin = skin;
            d->enableRemoteControlMode(skin->isRemoteControl() || d->settings->isEnabled(GlobalSettings::RemoteOverride));
            interface = d->loadQmlSkin(url);
        }
    }

    if (!interface) {
        d->enableRemoteControlMode(false);
        interface = d->loadSkinSelector();
    }

    return interface;
}

#include "skinruntime.moc"
