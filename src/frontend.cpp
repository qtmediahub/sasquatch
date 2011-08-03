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

#include "frontend.h"
#include "backend.h"

#include <QtGui>
#include <QDebug>

#ifdef SCENEGRAPH
#include <QSGItem>
#include <QSGView>
#else
#include <QtDeclarative>
#endif

#ifdef GLVIEWPORT
#include <QGLWidget>
#endif

#include "widgetwrapper.h"

#include "qmh-config.h"
#include "skin.h"
#include "qmh-util.h"
#include "dirmodel.h"
#include "media/playlist.h"
#include "file.h"
#include "media/mediamodel.h"
#include "media/mediascanner.h"
#include "actionmapper.h"
#include "rpc/rpcconnection.h"

#if defined(QMLJSDEBUGGER) && QT_VERSION < 0x040800

#include <qt_private/qdeclarativedebughelper_p.h>

#if !defined(NO_JSDEBUGGER)
#include <jsdebuggeragent.h>
#endif
#if !defined(NO_QMLOBSERVER)
#include <qdeclarativeviewobserver.h>
#endif

// Enable debugging before any QDeclarativeEngine is created
struct QmlJsDebuggingEnabler
{
    QmlJsDebuggingEnabler()
    {
        QDeclarativeDebugHelper::enableDebugging();
    }
};

// Execute code in constructor before first QDeclarativeEngine is instantiated
static QmlJsDebuggingEnabler enableDebuggingHelper;

#endif // QMLJSDEBUGGER

class DeclarativeView : public QDeclarativeView {
    Q_OBJECT
public:
    DeclarativeView();
    void setSource(const QUrl &url);

protected:
    void paintEvent(QPaintEvent *event);
    void timerEvent(QTimerEvent *event);

public slots:
    void handleSourceChanged();
    void handleStatusChanged(QDeclarativeView::Status status);

signals:
    void fpsCap(int);

private:
    int frameCount;
    int timeSigma;
    QElapsedTimer frameTimer;
    QUrl url;
};

DeclarativeView::DeclarativeView()
    : QDeclarativeView(0),
      frameCount(0)
{
    startTimer(1000);
    connect(this, SIGNAL(statusChanged(QDeclarativeView::Status)), this, SLOT(handleStatusChanged(QDeclarativeView::Status)));
}

void DeclarativeView::setSource(const QUrl &url)
{
    this->url = url;
    QMetaObject::invokeMethod(this, "handleSourceChanged", Qt::QueuedConnection);
}

void DeclarativeView::paintEvent(QPaintEvent *event)
{
    frameTimer.restart();
    QGraphicsView::paintEvent(event);
    timeSigma += frameTimer.elapsed();
    ++frameCount;
}

void DeclarativeView::timerEvent(QTimerEvent *event)
{
    if (timeSigma) {
        int cap = 1000*frameCount/timeSigma;
        timeSigma = frameCount = 0;
        emit fpsCap(cap);
    }
    QDeclarativeView::timerEvent(event);
}

void DeclarativeView::handleSourceChanged()
{
    QDeclarativeView::setSource(url);
}

void DeclarativeView::handleStatusChanged(QDeclarativeView::Status status)
{
    if (status == QDeclarativeView::Ready) {
        activateWindow();
    }
}

class FrontendPrivate : public QObject
{
    Q_OBJECT
public:
    FrontendPrivate(Frontend *p);
    ~FrontendPrivate();

public slots:
    bool setSkin(const QString &name);
    void initializeSkin(const QUrl &url);
    void resetLanguage();

    void initialize();

    void toggleFullScreen();
    void showFullScreen();
    void showNormal();
    void activateWindow();

    void grow();
    void shrink();

    void handleFPSCapChanged(int);

public:
    const QRect defaultGeometry;
    bool overscanWorkAround;
    bool attemptingFullScreen;

    int fpsCap;
    QTranslator frontEndTranslator;
    Skin *skin;
    QWidget *skinWidget;
    QDeclarativeContext *rootContext;
    Frontend *q;
};

FrontendPrivate::FrontendPrivate(Frontend *p)
    : QObject(p),
      //Leave space for Window decoration!
      defaultGeometry(0, 0, 1080, 720),
      overscanWorkAround(Config::isEnabled("overscan", false)),
      attemptingFullScreen(Config::isEnabled("fullscreen", true)),
      fpsCap(0),
      skinWidget(0),
      rootContext(0),
      q(p)
{
    QPixmapCache::setCacheLimit(Config::value("cacheSize", 0)*1024);

    qApp->setOverrideCursor(Qt::BlankCursor);
    qApp->installTranslator(&frontEndTranslator);
    QMetaObject::invokeMethod(this, "initialize", Qt::QueuedConnection);

    qRegisterMetaType<QModelIndex>();

    //Declarative is a hard dependency at present in any case
    // register dataproviders to QML
    qmlRegisterUncreatableType<ActionMapper>("ActionMapper", 1, 0, "ActionMapper", "For enums. For methods use actionmap global variable");
    qmlRegisterType<DirModel>("DirModel", 1, 0, "DirModel");
    qmlRegisterType<File>("File", 1, 0, "File");
    qmlRegisterType<Playlist>("Playlist", 1, 0, "Playlist");
    qmlRegisterType<MediaModel>("MediaModel", 1, 0, "MediaModel");
    qmlRegisterType<RpcConnection>("RpcConnection", 1, 0, "RpcConnection");
}

FrontendPrivate::~FrontendPrivate()
{
    Config::setEnabled("fullscreen", attemptingFullScreen);
    Config::setValue("skin", skin->name());
    Config::setEnabled("overscan", overscanWorkAround);

    Config::setValue("desktop-id", qApp->desktop()->screenNumber(skinWidget));

    if (!attemptingFullScreen)
        Config::setValue("window-geometry", skinWidget->geometry());
    else if (overscanWorkAround)
        Config::setValue("overscan-geometry", skinWidget->geometry());

    delete skinWidget;
}

void FrontendPrivate::initialize()
{
    setSkin(Config::value("skin", "").toString());
}

bool FrontendPrivate::setSkin(const QString &name)
{
    static QSize nativeResolution = qApp->desktop()->screenGeometry().size();
    static QString nativeResolutionString = Config::value("native-res-override", QString("%1x%2").arg(nativeResolution.width()).arg(nativeResolution.height()));
    //http://en.wikipedia.org/wiki/720p
    //1440, 1080, 720, 576, 480, 360, 240
    static QHash<QString, QString> resolutionHash;
    resolutionHash["1440"] = "2560x1440";
    resolutionHash["1080"] = "1920x1080";
    resolutionHash["720"] = "1280x720";

    Backend *backend = Backend::instance();
    Skin *newSkin = 0;
    Skin *defaultSkin = 0;
    QString defaultSkinName = Config::value("default-skin", "confluence").toString();

    foreach (QObject *o, backend->skins()) {
        Skin *s = qobject_cast<Skin*>(o);
        if (s->name() == name)
            newSkin = s;
        if (s->name() == defaultSkinName)
            defaultSkin = s;
    }

    if (!newSkin)
        newSkin = defaultSkin;

    if (!newSkin) {
        qDebug() << "Failed to set skin:" << name;
        return false;
    }

    QFile skinConfig(newSkin->config());
    if (skinConfig.open(QIODevice::ReadOnly)) {
        QHash<QString, QString> fileForResolution;
        QTextStream skinStream(&skinConfig);
        while (!skinStream.atEnd()) {
            QStringList resolutionToFile = skinStream.readLine().split(":");
            if (resolutionToFile.count() == 2) {
                QString resolution =
                        resolutionHash.contains(resolutionToFile.at(0))
                        ? resolutionHash[resolutionToFile.at(0)]
                        : resolutionToFile.at(0);
                fileForResolution[resolution] = resolutionToFile.at(1);
            } else {
                qWarning() << "bad line in skin configuration";
            }
        }

        QString urlPath =
                fileForResolution.contains(nativeResolutionString)
                ? fileForResolution[nativeResolutionString]
                : fileForResolution[Config::value("fallback-resolution", "default").toString()];

        skin = newSkin;

        initializeSkin(QUrl::fromLocalFile(skin->path() % "/" % urlPath));
    } else {
        qWarning() << "Can't read" << newSkin->name();
        return false;
    }
    return true;
}

void FrontendPrivate::initializeSkin(const QUrl &targetUrl)
{
    if (targetUrl.isEmpty() || !targetUrl.isValid())
        qFatal("You are explicitly forcing a broken url on the skin system");

    if (skinWidget) {
        Config::setValue("desktop-id", qApp->desktop()->screenNumber(skinWidget));
        if (!attemptingFullScreen)
            Config::setValue("window-geometry", skinWidget->geometry());
        else if (overscanWorkAround)
            Config::setValue("overscan-geometry", skinWidget->geometry());
        delete skinWidget;
        skinWidget = 0;
    }

    QPixmapCache::clear();

    if (targetUrl.path().right(3) == "qml") {
#ifdef SCENEGRAPH
        QSGView *declarativeWidget = new QSGView;
        declarativeWidget->setResizeMode(QSGView::SizeRootObjectToView);
#else
        DeclarativeView *declarativeWidget = new DeclarativeView;

        Utils::optimizeGraphicsViewAttributes(declarativeWidget);
        declarativeWidget->setResizeMode(QDeclarativeView::SizeRootObjectToView);

        if (Config::isEnabled("use-gl", true)) {
#ifdef GLVIEWPORT
            QGLWidget *viewport = new QGLWidget(declarativeWidget);
            //Why do I have to set this here?
            //Why can't I set it in the WidgetWrapper?
            Utils::optimizeWidgetAttributes(viewport, false);

            declarativeWidget->setViewport(viewport);
#endif //GLVIEWPORT
            declarativeWidget->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
        } else {
            declarativeWidget->setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);
        }
#endif //SCENEGRAPH
        //QSGEngine is not an equivalent class, this class holds for both
        QDeclarativeEngine *engine = declarativeWidget->engine();
        QObject::connect(engine, SIGNAL(quit()), qApp, SLOT(quit()));

        Backend::instance()->registerDeclarativeFrontend(declarativeWidget, skin);

        engine->addPluginPath(Backend::instance()->resourcePath() % "/lib");
        engine->addImportPath(Backend::instance()->resourcePath() % "/imports");
        engine->addImportPath(Backend::instance()->basePath() % "/imports");
        engine->addImportPath(skin->path());

        resetLanguage();
        skinWidget = new WidgetWrapper(declarativeWidget);
        connect(skinWidget, SIGNAL(grow()), this, SLOT(grow()));
        connect(skinWidget, SIGNAL(shrink()), this, SLOT(shrink()));
        connect(skinWidget, SIGNAL(toggleFullScreen()), this, SLOT(toggleFullScreen()));
        //FIXME?: item should have correct geometry
        //on QML parsing: work around several issues
        //Ovi maps inability to resize
        q->show();
        QApplication::processEvents();

        rootContext = declarativeWidget->rootContext();
        declarativeWidget->setSource(targetUrl);

        connect(declarativeWidget, SIGNAL(fpsCap(int)), SLOT(handleFPSCapChanged(int)));
    }
}

void FrontendPrivate::resetLanguage()
{
    Backend *backend = Backend::instance();
    QString language = backend->language();

    //FIXME: this clearly needs some heuristics
    frontEndTranslator.load(skin->path() % "/confluence/translations/" % "confluence_" % language % ".qm");
}

void FrontendPrivate::showFullScreen()
{
    attemptingFullScreen = true;
    overscanWorkAround = Config::isEnabled("overscan", false);

    if (overscanWorkAround) {
        QRect geometry = Config::value("overscan-geometry", defaultGeometry);
        geometry.moveCenter(qApp->desktop()->availableGeometry().center());

        skinWidget->setGeometry(geometry);
        skinWidget->setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
        skinWidget->setWindowState(Qt::WindowNoState);
        skinWidget->show();
    } else {
        skinWidget->showFullScreen();
    }

    QTimer::singleShot( 1, this, SLOT(activateWindow()));
}

void FrontendPrivate::showNormal()
{
    attemptingFullScreen = overscanWorkAround = false;

    skinWidget->setWindowFlags(Qt::Window);
    skinWidget->setGeometry(Config::value("window-geometry", defaultGeometry));
    skinWidget->showNormal();

    QTimer::singleShot( 1, this, SLOT(activateWindow()));
}

void FrontendPrivate::activateWindow()
{
    //Invoking this by adding it to the even queue doesn't work?
    skinWidget->activateWindow();
}

void FrontendPrivate::grow()
{
    if (!attemptingFullScreen)
        return;

    const QRect newGeometry = skinWidget->geometry().adjusted(-1,-1,1,1);

    const QSize desktopSize = qApp->desktop()->screenGeometry(skinWidget).size();
    if ((newGeometry.width() > desktopSize.width())
            || (newGeometry.height() > desktopSize.height())) {
        Config::setEnabled("overscan", false);
        showFullScreen();
    } else {
        skinWidget->setGeometry(newGeometry);
    }
}

void FrontendPrivate::shrink()
{
    if (!attemptingFullScreen)
        return;

    if (!overscanWorkAround) {
        Config::setEnabled("overscan");
        showFullScreen();
    }
    skinWidget->setGeometry(skinWidget->geometry().adjusted(1,1,-1,-1));
}

void FrontendPrivate::handleFPSCapChanged(int fpsCap)
{
    this->fpsCap = fpsCap;
    QMetaObject::invokeMethod(q, "framerateCapChanged");
}

void FrontendPrivate::toggleFullScreen()
{
    if (attemptingFullScreen) {
        Config::setValue("overscan-geometry", skinWidget->geometry());
        showNormal();
    } else {
        Config::setValue("window-geometry", skinWidget->geometry());
        showFullScreen();
    }
}

Frontend::Frontend(QObject *p)
    : QObject(p),
      d(new FrontendPrivate(this)) 
{
}

Frontend::~Frontend()
{
    delete d;
    d = 0;
}

bool Frontend::transforms() const
{
#ifdef GL
    return (QGLFormat::hasOpenGL() && Config::isEnabled("transforms", true));
#else
    return false;
#endif
}

int Frontend::framerateCap() const
{
    return d->fpsCap;
}

void Frontend::show()
{
    if (d->attemptingFullScreen) {
        d->showFullScreen();
    } else {
        d->showNormal();
    }
}

bool Frontend::setSkin(const QString &name)
{
    return d->setSkin(name);
}

void Frontend::addImportPath(const QString &path)
{
    if (QFile::exists(path))
        d->rootContext->engine()->addImportPath(path);
}

#include "frontend.moc"
