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

#include <QDebug>
#include <QFileInfo>
#include <QList>
#include <QVariant>

#ifdef SCENEGRAPH
#include <QSGItem>
#include <QSGView>
#else
#include <QDeclarativeItem>
#include <QDeclarativeView>
#endif
#include <QDeclarativeContext>
#include <QDeclarativeEngine>
#include <QDeclarativeComponent>
#include <QApplication>
#include <QTimer>
#include <QTranslator>

#ifdef GLVIEWPORT
#include <QGLWidget>
#endif

#include "qml-extensions/actionmapper.h"
#include "qml-extensions/mediaplayerhelper.h"
#include "qml-extensions/trackpad.h"
#include "rpc/rpcconnection.h"
#include "qmh-config.h"
#include "skin.h"

#include "dataproviders/proxymodel.h"
#include "dataproviders/dirmodel.h"
#include "qml-extensions/qmlfilewrapper.h"
#include "dataproviders/playlist.h"
#include "qmhplugin.h"

class QMLUtils : public QObject
{
    Q_OBJECT
public:
    QMLUtils(QObject *pQmlContainer) : QObject(pQmlContainer), qmlContainer(pQmlContainer) { /**/ }
    Q_INVOKABLE void applyWebViewFocusFix(QDeclarativeItem *item); // See https://bugs.webkit.org/show_bug.cgi?id=51094
    Q_INVOKABLE QObject* focusItem() const;
private:
    QObject *qmlContainer;
};

void QMLUtils::applyWebViewFocusFix(QDeclarativeItem *item) // See https://bugs.webkit.org/show_bug.cgi?id=51094
{
    item->setFlag(QGraphicsItem::ItemIsFocusScope, true);
    QList<QGraphicsItem *> children = item->childItems();
    for (int i = 0; i < children.count(); i++) {
        if (QGraphicsWidget *maybeWebView = qgraphicsitem_cast<QGraphicsWidget *>(children[i])) {
            if (maybeWebView->inherits("QGraphicsWebView"))
                maybeWebView->setFocus();
        }
    }
}

QObject* QMLUtils::focusItem() const {
#ifdef SCENEGRAPH
    return qobject_cast<QSGView*>(qmlContainer)->activeFocusItem();
#else
    return qgraphicsitem_cast<QGraphicsObject *>(qobject_cast<QDeclarativeView*>(qmlContainer)->scene()->focusItem());
#endif
}

class WidgetWrapper : public QWidget
{
    Q_OBJECT
public:
    WidgetWrapper(QWidget *prey);

protected:
    void paintEvent(QPaintEvent *e);
    void resizeEvent(QResizeEvent *e);

signals:
    void shrink();
    void grow();
    void toggleFullScreen();

public slots:
    void handleResize();
    void resetUI();

private:
    QTimer resizeSettleTimer;
    QWidget *m_prey;
};

WidgetWrapper::WidgetWrapper(QWidget *prey)
    : QWidget(0),
      m_prey(prey)
{
    m_prey->setParent(this);

    installEventFilter(Backend::instance());

    setAttribute(Qt::WA_OpaquePaintEvent);
    setAttribute(Qt::WA_NoSystemBackground);

    new QShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::ALT + Qt::Key_Backspace), this, SLOT(resetUI()));
    new QShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::ALT + Qt::Key_Down), this, SIGNAL(shrink()));
    new QShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::ALT + Qt::Key_Up), this, SIGNAL(grow()));
    new QShortcut(QKeySequence(Qt::ALT + Qt::Key_Return), this, SIGNAL(toggleFullScreen()));

    resizeSettleTimer.setSingleShot(true);

    connect(&resizeSettleTimer, SIGNAL(timeout()), this, SLOT(handleResize()));
}

void WidgetWrapper::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e)
    QPainter p(this);
    p.fillRect(rect(), Qt::black);
}

void WidgetWrapper::resizeEvent(QResizeEvent *e)
{
    Q_UNUSED(e)
    resizeSettleTimer.start(300);
}

void WidgetWrapper::handleResize()
{
    m_prey->setFixedSize(size());

    QGraphicsView *gv = qobject_cast<QGraphicsView*>(m_prey);
    if (gv && Config::isEnabled("scale-ui", false)) {
        gv->resetMatrix();
        //Needs to be scaled by res of top level qml file
        gv->scale(qreal(width())/1280, qreal(height())/720);
    }
}

void WidgetWrapper::resetUI()
{
    QDeclarativeView *declarativeWidget = qobject_cast<QDeclarativeView*>(m_prey);
    if (declarativeWidget) {
        QObject* coreObject = declarativeWidget->rootObject();
        coreObject->setProperty("state", "");
        //coreObject->setProperty("focus", true);
        QMetaObject::invokeMethod(coreObject, "resetFocus");
    }
}

class FrontendPrivate : public QObject
{
    Q_OBJECT
public:
    FrontendPrivate(Frontend *p);
    ~FrontendPrivate();

public slots:
    void setSkin(const QString &name);
    void initializeSkin(const QUrl &url);
    void resetLanguage();

    void initialize();

    void toggleFullScreen();
    void showFullScreen();
    void showNormal();

    void grow();
    void shrink();

public:
    const QRect defaultGeometry;
    bool overscanWorkAround;
    bool attemptingFullScreen;

    QTranslator frontEndTranslator;
    Skin *skin;
    ActionMapper *actionMap;
    MediaPlayerHelper *mediaPlayerHelper;
    Trackpad *trackpad;
    QWidget *skinWidget;
    Frontend *pSelf;
};

FrontendPrivate::FrontendPrivate(Frontend *p)
    : QObject(p),
      //Leave space for Window decoration!
      defaultGeometry(0, 0, 1080, 720),
      overscanWorkAround(Config::isEnabled("overscan", false)),
      attemptingFullScreen(Config::isEnabled("fullscreen", true)),
      actionMap(0),
      mediaPlayerHelper(0),
      trackpad(0),
      skinWidget(0),
      pSelf(p)
{
    qApp->setOverrideCursor(Qt::BlankCursor);
    qApp->installTranslator(&frontEndTranslator);
    QMetaObject::invokeMethod(this, "initialize", Qt::QueuedConnection);
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

void FrontendPrivate::setSkin(const QString &name)
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
        //Ultimate fallback
        newSkin = new Skin(QFileDialog::getOpenFileName(0, tr("Select a suitable skin")), this);
    }

    QFile skinConfig(newSkin->config());
    if (skinConfig.open(QIODevice::ReadOnly))
    {
        QHash<QString, QString> fileForResolution;
        QTextStream skinStream(&skinConfig);
        while(!skinStream.atEnd())
        {
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
    }
    else {
        qWarning() << "Can't read" << newSkin->name();
    }
    pSelf->show();
}

void FrontendPrivate::initializeSkin(const QUrl &targetUrl)
{
    if (targetUrl.isEmpty() || !targetUrl.isValid())
        qFatal("You are explicitly forcing a broken url on the skin system");

    if (skinWidget)
    {
        Config::setValue("desktop-id", qApp->desktop()->screenNumber(skinWidget));
        if (!attemptingFullScreen)
            Config::setValue("window-geometry", skinWidget->geometry());
        else if (overscanWorkAround)
            Config::setValue("overscan-geometry", skinWidget->geometry());
        delete skinWidget;
        skinWidget = 0;
    }

    if (targetUrl.path().right(3) == "qml")
    {
#ifdef SCENEGRAPH
        QSGView *declarativeWidget = new QSGView;
        declarativeWidget->setResizeMode(QSGView::SizeRootObjectToView);
#else
        QDeclarativeView *declarativeWidget = new QDeclarativeView;
        declarativeWidget->setAutoFillBackground(false);
        declarativeWidget->setAttribute(Qt::WA_OpaquePaintEvent);
        declarativeWidget->setAttribute(Qt::WA_NoSystemBackground);


        if (Config::isEnabled("smooth-scaling", true))
            declarativeWidget->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform | QPainter::TextAntialiasing);

        declarativeWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        declarativeWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        declarativeWidget->setFrameStyle(0);
        declarativeWidget->setOptimizationFlags(QGraphicsView::DontSavePainterState);
        declarativeWidget->scene()->setItemIndexMethod(QGraphicsScene::NoIndex);
        declarativeWidget->setResizeMode(QDeclarativeView::SizeRootObjectToView);

        if (Config::isEnabled("use-gl", true))
        {
#ifdef GLVIEWPORT
            QGLWidget *viewport = new QGLWidget(declarativeWidget);
            viewport->setAttribute(Qt::WA_OpaquePaintEvent);
            viewport->setAttribute(Qt::WA_NoSystemBackground);
            viewport->setAutoFillBackground(false);
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

        // register dataproviders to QML
        qmlRegisterType<ActionMapper>("ActionMapper", 1, 0, "ActionMapper");
        qmlRegisterType<QMHPlugin>("QMHPlugin", 1, 0, "QMHPlugin");
        qmlRegisterType<ProxyModel>("ProxyModel", 1, 0, "ProxyModel");
        qmlRegisterType<ProxyModelItem>("ProxyModel", 1, 0, "ProxyModelItem");
        qmlRegisterType<DirModel>("DirModel", 1, 0, "DirModel");
        qmlRegisterType<QMLFileWrapper>("QMLFileWrapper", 1, 0, "QMLFileWrapper");
        qmlRegisterType<Playlist>("Playlist", 1, 0, "Playlist");
        qmlRegisterType<RpcConnection>("RpcConnection", 1, 0, "RpcConnection");

        actionMap = new ActionMapper(declarativeWidget);
        mediaPlayerHelper = new MediaPlayerHelper(declarativeWidget);
        trackpad = new Trackpad(declarativeWidget);

        RpcConnection *connection = new RpcConnection(RpcConnection::Server, QHostAddress::Any, 1234, declarativeWidget);
        connection->registerObject(actionMap);
        connection->registerObject(mediaPlayerHelper);
        connection->registerObject(trackpad);

        // attach global context properties
        engine->rootContext()->setContextProperty("actionmap", actionMap);
        engine->rootContext()->setContextProperty("mediaPlayerHelper", mediaPlayerHelper);
        engine->rootContext()->setContextProperty("trackpad", trackpad);
        engine->rootContext()->setContextProperty("frontend", pSelf);
        engine->rootContext()->setContextProperty("utils", new QMLUtils(declarativeWidget));
        engine->rootContext()->setContextProperty("skin", skin);
        engine->rootContext()->setContextProperty("backend", Backend::instance());

        engine->addPluginPath(Backend::instance()->resourcePath() % "/lib");
        engine->addImportPath(Backend::instance()->resourcePath() % "/imports");
        engine->addImportPath(skin->path());

        declarativeWidget->rootContext()->setContextProperty("config", Config::instance());

        foreach (QMHPlugin *plugin, Backend::instance()->allEngines())
            plugin->registerPlugin(declarativeWidget->rootContext());

        //Really need to have this dude go out of scope that quickly?
        {
            const QMetaObject &PluginMO = QMHPlugin::staticMetaObject;
            int enumIndex = PluginMO.indexOfEnumerator("PluginRole");
            QMetaEnum roleEnum = PluginMO.enumerator(enumIndex);

            foreach (QObject *p, Backend::instance()->allEngines()) {
                QMHPlugin *plugin = qobject_cast<QMHPlugin *>(p);
                if (plugin && plugin->role() < QMHPlugin::SingletonRoles) {
                    declarativeWidget->rootContext()->setContextProperty(QString(roleEnum.valueToKey(plugin->role())).toLower() + "Engine", plugin);
                }
            }
        }

        resetLanguage();
        skinWidget = new WidgetWrapper(declarativeWidget);
        connect(skinWidget, SIGNAL(grow()), this, SLOT(grow()));
        connect(skinWidget, SIGNAL(shrink()), this, SLOT(shrink()));
        connect(skinWidget, SIGNAL(toggleFullScreen()), this, SLOT(toggleFullScreen()));
        declarativeWidget->setSource(targetUrl);
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

    skinWidget->activateWindow();
}

void FrontendPrivate::showNormal()
{
    attemptingFullScreen = overscanWorkAround = false;

    skinWidget->setWindowFlags(Qt::Window);
    skinWidget->setGeometry(Config::value("window-geometry", defaultGeometry));
    skinWidget->showNormal();

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
    }
    else {
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

void FrontendPrivate::toggleFullScreen()
{
    if (attemptingFullScreen) {
        Config::setValue("overscan-geometry", skinWidget->geometry());
        showNormal();
    }
    else {
        Config::setValue("window-geometry", skinWidget->geometry());
        showFullScreen();
    }
}

Frontend::Frontend(QObject *p)
    : QObject(p),
      d(new FrontendPrivate(this)) { /*no impl*/ }

Frontend::~Frontend()
{
    delete d;
    d = 0;
}

void Frontend::show()
{
    if (d->attemptingFullScreen) {
        d->showFullScreen();
    } else {
        d->showNormal();
    }
}

#include "frontend.moc"
