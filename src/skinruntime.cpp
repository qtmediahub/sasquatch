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
#include <QSGItem>
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

#include "qmh-config.h"
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
#ifndef NO_DBUS
#include "mediaplayer_dbus.h"
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
    SkinRuntimePrivate(SkinRuntime *p);
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
    SkinRuntime *q;
};

SkinRuntimePrivate::SkinRuntimePrivate(SkinRuntime *p)
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
      q(p)
{
#ifndef NO_DBUS
    dbusRegistration = QDBusConnection::sessionBus().registerService(QMH_DBUS_SERVICENAME);
    if (!dbusRegistration) {
        qDebug() << "Can't seem to register dbus service:" << QDBusConnection::sessionBus().lastError().message();
    }
#endif

    QPixmapCache::setCacheLimit(Config::value("cacheSize", 0)*1024);

#ifdef GL
    //Can't fool with the default format when dealing with the Tegra for some reason
    if (Config::isEnabled("adjust-gl-format", true)) {
        QGLFormat format;
        //explicitly set options
        format.setDoubleBuffer(true);
        format.setSampleBuffers(true);
        format.setAlpha(true);
        format.setDepth(false);
        format.setStencil(false);
        format.setSwapInterval(1);
        QGLFormat::setDefaultFormat(format);
    }
#endif //GL

    foreach (const QString &resourcePath, LibraryInfo::resourcePaths()) {
        QString dejavuPath(resourcePath % "/3rdparty/dejavu-fonts-ttf-2.32/ttf/");
        if (QDir(dejavuPath).exists()) {
            qDebug() << "Using the application specified dejavu font";
            QFontDatabase::addApplicationFont(dejavuPath % "DejaVuSans.ttf");
            QFontDatabase::addApplicationFont(dejavuPath % "DejaVuSans-Bold.ttf");
            QFontDatabase::addApplicationFont(dejavuPath % "DejaVuSans-Oblique.ttf");
            QFontDatabase::addApplicationFont(dejavuPath % "DejaVuSans-BoldOblique.ttf");
            QApplication::setFont(QFont("DejaVu Sans"));
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
#ifndef NO_DBUS
    if (Config::value("overlay-mode", false)) {
        qmlRegisterType<MediaPlayerDbus>("OverlayModeMediaPlayer", 1, 0, "OverlayModeMediaPlayer");
    }
#endif

#ifdef QMH_AVAHI
    if (Config::isEnabled("avahi", true)) {
        QAvahiServiceBrowserModel *model = new QAvahiServiceBrowserModel(this);
        model->setAutoResolve(true);
        QAvahiServiceBrowserModel::Options options = QAvahiServiceBrowserModel::NoOptions;
        if (Config::isEnabled("avahi-hide-ipv6"), true)
            options |= QAvahiServiceBrowserModel::HideIPv6;
        if (Config::isEnabled("avahi-hide-local", true) && !Config::isEnabled("testing", false))
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
    Config::setValue("skin", currentSkin->name());
}

#ifndef SCENEGRAPH
static void optimizeGraphicsViewAttributes(QGraphicsView *view)
{
    if (Config::isEnabled("smooth-scaling", true))
        view->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform | QPainter::TextAntialiasing);

    view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setFrameStyle(0);
    view->setOptimizationFlags(QGraphicsView::DontSavePainterState);
    view->scene()->setItemIndexMethod(QGraphicsScene::NoIndex);
    view->setCacheMode(QGraphicsView::CacheNone);
}
#endif

QObject *SkinRuntimePrivate::loadQmlSkin(const QUrl &targetUrl, QObject *window)
{
    QPixmapCache::clear();

    DeclarativeView *declarativeWidget = new DeclarativeView;

#ifdef SCENEGRAPH
    declarativeWidget->setResizeMode(QSGView::SizeRootObjectToView);
#else
    optimizeGraphicsViewAttributes(declarativeWidget);
    declarativeWidget->setResizeMode(QDeclarativeView::SizeRootObjectToView);

    if (Config::isEnabled("use-gl", true)) {
#ifdef GLVIEWPORT
        if (Config::isEnabled("use-glviewport", true)) {
            QGLWidget *viewport = new QGLWidget();
            if (Config::isEnabled("overlay-mode", false)) {
                viewport->setAttribute(Qt::WA_TranslucentBackground);
            }
            viewport->qglClearColor(Qt::transparent);
            declarativeWidget->setViewport(viewport);
        }
#endif //GLVIEWPORT
        declarativeWidget->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    } else {
        declarativeWidget->viewport()->setAutoFillBackground(false);
        declarativeWidget->setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);
    }
    declarativeWidget->viewport()->setAttribute(Qt::WA_OpaquePaintEvent);
#endif //SCENEGRAPH
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
    runtime->insert("config", qVariantFromValue(static_cast<QObject *>(Config::instance())));
    runtime->insert("window", qVariantFromValue(static_cast<QObject *>(window)));
    runtime->insert("view", qVariantFromValue(static_cast<QObject *>(declarativeWidget)));
    runtime->insert("cursor", qVariantFromValue(static_cast<QObject *>(new CustomCursor(declarativeWidget))));
    runtime->insert("skin", qVariantFromValue(static_cast<QObject *>(currentSkin)));
    runtime->insert("file", qVariantFromValue(static_cast<QObject *>(new File(this))));
    runtime->insert("remoteSessionsModel", qVariantFromValue(static_cast<QObject *>(remoteSessionsModel)));

    declarativeWidget->rootContext()->setContextProperty("runtime", runtime);

    const QString videoPlayer = Config::value("overlay-mode", false) ? "overlaymode" : "mobility";
    foreach (const QString &qmlImportPath, LibraryInfo::qmlImportPaths()) {
        engine->addImportPath(qmlImportPath);
        engine->addImportPath(qmlImportPath % "/QtMediaHub/components/media/" % videoPlayer); // ## is this correct?
    }
    engine->addImportPath(currentSkin->path());

    declarativeWidget->setSource(targetUrl);

    return declarativeWidget;
}

SkinRuntime::SkinRuntime(QObject *p)
    : QObject(p),
      d(new SkinRuntimePrivate(this))
{
}

SkinRuntime::~SkinRuntime()
{
    delete d;
    d = 0;
}

QObject *SkinRuntime::create(Skin *skin, QObject *window)
{
    QSize nativeResolution = qApp->desktop()->screenGeometry().size();
    QString nativeResolutionString = Config::value("native-res-override", QString("%1x%2").arg(nativeResolution.width()).arg(nativeResolution.height()));

    QUrl url = skin->urlForResolution(nativeResolutionString, Config::value("fallback-resolution", "default").toString());
    if (!url.isValid()) {
        qWarning() << "Error loading skin " << skin->name();
        return 0;
    }

    if (skin->type(url) != Skin::Qml)
        return 0;

    d->currentSkin = skin;
    d->enableRemoteControlMode(skin->isRemoteControl() || Config::isEnabled("remote-override", false));
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

    mediaServer = new MediaServer(this);
    rpcConnection = new RpcConnection(RpcConnection::Server, QHostAddress::Any, 1234, this);
    mediaPlayerRpc = new MediaPlayerRpc(this);
    mediaPlayerRpc->setObjectName("qmhmediaplayer");
    trackpad = new Trackpad(this);
    trackpad->setObjectName("trackpad");
    actionMapper = new ActionMapper(this);
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
