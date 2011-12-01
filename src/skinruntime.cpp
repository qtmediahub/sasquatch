/****************************************************************************

This file is part of the QtMediaHub project on http://www.qtmediahub.com

Copyright (c) 2011 Nokia Corporation and/or its subsidiary(-ies).*
All rights reserved.

Contact:  Girish Ramakrishnan girish@forwardbias.in
Contact:  Nokia Corporation donald.carr@nokia.com
Contact:  Nokia Corporation johannes.zellner@nokia.com

You may use this file under the terms of the BSD license as follows:

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
* Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
* Neither the name of Nokia Corporation and its Subsidiary(-ies) nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

****************************************************************************/

#include "skinruntime.h"
#include "mediaserver.h"
#include "tarfileengine.h"

#include <QtGui>
#include <QDebug>

#ifdef QMH_AVAHI
#include "qavahiservicebrowsermodel.h"
#else
#include "staticservicebrowsermodel.h"
#endif

#include "declarativeview.h"

#ifdef SCENEGRAPH
#include <QApplication>
#include <QDesktopWidget>
#else
#include <QtDeclarative>
#endif

#ifndef NO_DBUS
#include <QDBusError>
#include <QDBusConnection>
#endif

#ifdef GL
#include <QGLFormat>
#endif

#ifdef GLVIEWPORT
#include <QGLWidget>
#endif

#include "dirmodel.h"
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

#ifdef MEDIAPLAYER_DBUS
#include "mediaplayerdbus.h"
#elif defined(MEDIAPLAYER_VLC)
#include "mediaplayervlc.h"
#else
#include "mediaplayerdummy.h"
#endif

#include "customcursor.h"
#include "httpserver/httpserver.h"
#include "inputcontext.h"

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
    QObject *loadQmlSkin(const QUrl &url, QObject *window);

    // TODO check if there is some better place
    void rpcSendInputMethodStart();
    void rpcSendInputMethodStop();

public:
    void enableRemoteControlMode(bool enable);

    bool dbusRegistration;
    bool remoteControlMode;
    MediaServer *mediaServer;
    ProcessManager *processManager;
    DeviceManager *deviceManager;
    PowerManager *powerManager;
    MediaPlayerRpc *mediaPlayerRpc;
    RpcConnection *rpcConnection;

    ActionMapper *actionMapper;
    Trackpad *trackpad;
    Skin *currentSkin;
    QAbstractItemModel *remoteSessionsModel;
    InputContext *inputContext;
    GlobalSettings *settings;
    SkinRuntime *q;
};

SkinRuntimePrivate::SkinRuntimePrivate(GlobalSettings *s, SkinRuntime *p)
    : QObject(p),
      dbusRegistration(false),
      remoteControlMode(true),
      mediaServer(0),
      processManager(0),
      deviceManager(0),
      powerManager(0),
      mediaPlayerRpc(0),
      rpcConnection(0),
      trackpad(0),
      remoteSessionsModel(0),
      inputContext(0),
      settings(s),
      q(p)
{
#ifndef NO_DBUS
    dbusRegistration = QDBusConnection::sessionBus().registerService(QMH_DBUS_SERVICENAME);
    if (!dbusRegistration) {
        qDebug() << "Can't seem to register dbus service:" << QDBusConnection::sessionBus().lastError().message();
    }
#endif

#ifdef GL
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
#ifdef SCENEGRAPH
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
    qmlRegisterType<DirModel>("DirModel", 1, 0, "DirModel");
    qmlRegisterType<Playlist>("Playlist", 1, 0, "Playlist");
    qmlRegisterType<MediaModel>("MediaModel", 1, 0, "MediaModel");
    qmlRegisterType<RpcConnection>("RpcConnection", 1, 0, "RpcConnection");
    qmlRegisterType<Settings>("Settings", 1, 0, "Settings");

    if (settings->isEnabled(GlobalSettings::OverlayMode)) {
#ifdef MEDIAPLAYER_DBUS
    qmlRegisterType<MediaPlayerDbus>("OverlayModeMediaPlayer", 1, 0, "OverlayModeMediaPlayer");
#elif defined(MEDIAPLAYER_VLC)
        qmlRegisterType<MediaPlayerVLC>("OverlayModeMediaPlayer", 1, 0, "OverlayModeMediaPlayer");
#else
        qmlRegisterType<MediaPlayerDummy>("OverlayModeMediaPlayer", 1, 0, "OverlayModeMediaPlayer");
#endif
    }

#ifdef QMH_AVAHI
    if (settings->isEnabled(GlobalSettings::Avahi)) {
        QAvahiServiceBrowserModel *model = new QAvahiServiceBrowserModel(this);
        model->setAutoResolve(true);
        QAvahiServiceBrowserModel::Options options = QAvahiServiceBrowserModel::NoOptions;
        if (settings->isEnabled(GlobalSettings::AvahiHideIPv6))
            options |= QAvahiServiceBrowserModel::HideIPv6;
        if (settings->isEnabled(GlobalSettings::AvahiHideLocal))
            options |= QAvahiServiceBrowserModel::HideLocal;
        model->browse("_qtmediahub._tcp", options);
        remoteSessionsModel = model;
    }
#else
    remoteSessionsModel = new StaticServiceBrowserModel(this);
#endif
}

SkinRuntimePrivate::~SkinRuntimePrivate()
{
    settings->setValue(GlobalSettings::Skin, currentSkin->name());
}

QObject *SkinRuntimePrivate::loadQmlSkin(const QUrl &targetUrl, QObject *window)
{
    DeclarativeView *declarativeWidget = new DeclarativeView(settings);

    QDeclarativeEngine *engine = declarativeWidget->engine();
    QObject::connect(engine, SIGNAL(quit()), qApp, SLOT(quit()));

    QDeclarativePropertyMap *runtime = new QDeclarativePropertyMap(declarativeWidget);
    if (!remoteControlMode) {
        runtime->insert("mediaScanner", qVariantFromValue(static_cast<QObject *>(mediaServer->mediaScanner())));
        runtime->insert("httpServer", qVariantFromValue(static_cast<QObject *>(mediaServer->httpServer())));
#ifndef SCENEGRAPH
        actionMapper->setRecipient(declarativeWidget);
        trackpad->setRecipient(declarativeWidget);
#endif //Fixme: Need to harden code for non-QWidget derived classes
        runtime->insert("actionMapper", qVariantFromValue(static_cast<QObject *>(actionMapper)));
        runtime->insert("trackpad", qVariantFromValue(static_cast<QObject *>(trackpad)));
        runtime->insert("mediaPlayerRpc", qVariantFromValue(static_cast<QObject *>(mediaPlayerRpc)));
        runtime->insert("processManager", qVariantFromValue(static_cast<QObject *>(processManager)));
        runtime->insert("deviceManager", qVariantFromValue(static_cast<QObject *>(deviceManager)));
        runtime->insert("powerManager", qVariantFromValue(static_cast<QObject *>(powerManager)));
    }
    runtime->insert("settings", qVariantFromValue(static_cast<QObject *>(settings)));
    runtime->insert("window", qVariantFromValue(static_cast<QObject *>(window)));
    runtime->insert("view", qVariantFromValue(static_cast<QObject *>(declarativeWidget)));
    runtime->insert("cursor", qVariantFromValue(static_cast<QObject *>(new CustomCursor(settings, declarativeWidget))));
    runtime->insert("skin", qVariantFromValue(static_cast<QObject *>(currentSkin)));
    runtime->insert("apps", qVariantFromValue(static_cast<QObject *>(new AppsManager(settings, this))));
    runtime->insert("file", qVariantFromValue(static_cast<QObject *>(new File(this))));
    runtime->insert("remoteSessionsModel", qVariantFromValue(static_cast<QObject *>(remoteSessionsModel)));

    declarativeWidget->rootContext()->setContextProperty("runtime", runtime);

    const QString mediaPlayer = settings->isEnabled(GlobalSettings::OverlayMode) ? "overlaymode" : "mobility";
    foreach (const QString &qmlImportPath, LibraryInfo::qmlImportPaths(settings)) {
        engine->addImportPath(qmlImportPath);
        engine->addImportPath(qmlImportPath % "/QtMediaHub/components/media/" % mediaPlayer); // ## is this correct?
    }
    engine->addImportPath(currentSkin->path());

    declarativeWidget->setSource(targetUrl);

    return declarativeWidget;
}

SkinRuntime::SkinRuntime(GlobalSettings *settings, QObject *p)
    : QObject(p),
      d(new SkinRuntimePrivate(settings, this))
{
}

SkinRuntime::~SkinRuntime()
{
    delete d;
    d = 0;
}

QObject *SkinRuntime::create(Skin *skin, QObject *window)
{
    const QSize res = d->settings->value(GlobalSettings::SkinResolution).toRect().size(); // TODO provide a toSize for Settings
    const QSize preferredResolution = res.isEmpty() ? qApp->desktop()->screenGeometry().size() : res;

    skin->parseManifest();

    QUrl url = skin->urlForResolution(preferredResolution);
    if (!url.isValid()) {
        qWarning() << "Error loading skin " << skin->name();
        return 0;
    }

    if (skin->type(url) != Skin::Qml)
        return 0;

    d->currentSkin = skin;
    d->enableRemoteControlMode(skin->isRemoteControl() || d->settings->isEnabled(GlobalSettings::RemoteOverride));
    return d->loadQmlSkin(url, window);
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
    rpcConnection = new RpcConnection(RpcConnection::Server, QHostAddress::Any, 1234, this);
    mediaPlayerRpc = new MediaPlayerRpc(this);
    mediaPlayerRpc->setObjectName("qmhmediaplayer");
    trackpad = new Trackpad(this);
    trackpad->setObjectName("trackpad");
    actionMapper = new ActionMapper(settings, this);
    actionMapper->setObjectName("qmhrpc");

    rpcConnection->registerObject(actionMapper);
    rpcConnection->registerObject(mediaPlayerRpc);
    rpcConnection->registerObject(trackpad);

    inputContext = new InputContext(this);
    connect(inputContext, SIGNAL(inputMethodStartRequested()), this, SLOT(rpcSendInputMethodStart()));
    connect(inputContext, SIGNAL(inputMethodStopRequested()), this, SLOT(rpcSendInputMethodStop()));

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

#include "skinruntime.moc"
