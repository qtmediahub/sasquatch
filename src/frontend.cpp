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

#include <QDeclarativeView>
#include <QDeclarativeEngine>
#include <QDeclarativeContext>
#include <QDeclarativeComponent>
#include <QApplication>
#include <QTimer>
#include <QTranslator>

#ifdef GLVIEWPORT
#include <QGLWidget>
#endif

#include "config.h"

class FrontendPrivate : public QObject
{
    Q_OBJECT
public:
    FrontendPrivate(Frontend *p);
    ~FrontendPrivate();
public slots:
    void handleResize();
public:
    QWidget *centralWidget;
    QTranslator *frontEndTranslator;
    QString skin;
    QTimer resizeSettleTimer;
    Frontend *pSelf;
};

FrontendPrivate::FrontendPrivate(Frontend *p)
    : QObject(p),
      centralWidget(0),
      frontEndTranslator(0),
      pSelf(p)
{
    resizeSettleTimer.setSingleShot(true);
    connect(&resizeSettleTimer, SIGNAL(timeout()), this, SLOT(handleResize()));
}

FrontendPrivate::~FrontendPrivate()
{
    Config::setValue("last-skin", skin);
    delete centralWidget;
    centralWidget = 0;
}

void FrontendPrivate::handleResize()
{
    if(centralWidget)
        centralWidget->setFixedSize(pSelf->size());
    QGraphicsView *gv = qobject_cast<QGraphicsView*>(centralWidget);
    if(gv && Config::isEnabled("scale-ui", false)) {
        gv->resetMatrix();
        //Needs to be scaled by res of top level qml file
        gv->scale(qreal(pSelf->width())/1280, qreal(pSelf->height())/720);
    }
}

Frontend::Frontend(QWidget *p)
    : QWidget(p),
      d(new FrontendPrivate(this))
{
    setGeometry(Config::value("windowGeometry", QRect(0, 0, 1080, 720)));
    setSkin(Config::value("last-skin", "").toString());
}

Frontend::~Frontend()
{
    Config::setValue("windowGeometry", geometry());
    delete d;
    d = 0;
    //Can't decide whether this is filthy or not
    Backend::destroy();
}

void Frontend::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e)
    QPainter p(this);
    p.fillRect(rect(), Qt::black);
}

void Frontend::resizeEvent(QResizeEvent *e)
{
    Q_UNUSED(e)
    d->resizeSettleTimer.start(300);
}

void Frontend::setSkin(const QString &name)
{
    //Maybe we will be able to influence this?
    static QSize nativeResolution = qApp->desktop()->screenGeometry(this).size();
    static QString nativeResolutionString = QString("%1x%2").arg(nativeResolution.width()).arg(nativeResolution.height());
    //http://en.wikipedia.org/wiki/720p
    //1440, 1080, 720, 576, 480, 360, 240
    static QHash<QString, QString> resolutionHash;
    resolutionHash["1440"] = "2560x1440";
    resolutionHash["1080"] = "1920x1080";
    resolutionHash["720"] = "1280x720";

    Backend *backend = Backend::instance();
    QString skinName(name);

    if(!backend->skins().contains(skinName)) {
        skinName = Config::value("default-skin", "confluence").toString();
    }

    QFile skinConfig(backend->skinPath() % "/" % skinName % "/" % skinName);
    if(!skinConfig.exists()) {
        qFatal("Something has gone horribly awry, you want for skins");
    }
    if(skinConfig.open(QIODevice::ReadOnly))
    {
        QHash<QString, QString> fileForResolution;
        QTextStream skinStream(&skinConfig);
        while(!skinStream.atEnd())
        {
            QStringList resolutionToFile = skinStream.readLine().split(":");
            QString resolution =
                resolutionHash.contains(resolutionToFile.at(0))
                ? resolutionHash[resolutionToFile.at(0)]
                : resolutionToFile.at(0);
            fileForResolution[resolution] = resolutionToFile.at(1);
        }

        QString urlPath =
            fileForResolution.contains(nativeResolutionString)
            ? fileForResolution[nativeResolutionString]
            : fileForResolution[Config::value("fallback-resolution", "default").toString()];

        d->skin = skinName;

        initialize(QUrl::fromLocalFile(Backend::instance()->skinPath() % "/" % skinName % "/" % urlPath));
    }
    else {
        qWarning() << "Can't read" << skinName;
    }
}

void Frontend::initialize(const QUrl &targetUrl)
{
    if(targetUrl.isEmpty() || !targetUrl.isValid())
        qFatal("You are explicitly forcing a broken url on the skin system");

    delete d->centralWidget;

    if(targetUrl.path().right(3) == "qml")
    {
        QDeclarativeView *centralWidget= new QDeclarativeView(this);
        QDeclarativeEngine *engine = centralWidget->engine();

        engine->addPluginPath(Backend::instance()->resourcePath() % "/lib");
        engine->addImportPath(Backend::instance()->resourcePath() % "/imports");
        engine->addImportPath(Backend::instance()->skinPath());

        if(Config::isEnabled("smooth-scaling", true))
            centralWidget->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform | QPainter::TextAntialiasing);

        centralWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        centralWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        centralWidget->setFrameStyle(0);
        centralWidget->setOptimizationFlags(QGraphicsView::DontSavePainterState);
        centralWidget->scene()->setItemIndexMethod(QGraphicsScene::NoIndex);
        centralWidget->setResizeMode(QDeclarativeView::SizeRootObjectToView);

        QObject::connect(engine, SIGNAL(quit()), qApp, SLOT(quit()));

#ifdef GLVIEWPORT
        centralWidget->setViewport(new QGLWidget());
#endif
#ifdef GL
        centralWidget->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
#else
        centralWidget->setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);
#endif
        Backend::instance()->initialize(engine);

        resetLanguage();
        centralWidget->setSource(targetUrl);

        d->centralWidget = centralWidget;
    }
}

void Frontend::resetLanguage()
{
    Backend *backend = Backend::instance();
    QString language = backend->language();

    delete d->frontEndTranslator;
    d->frontEndTranslator = new QTranslator(this);
    //FIXME: this clearly needs some heuristics
    d->frontEndTranslator->load(backend->skinPath() % "/confluence/translations/" % "confluence_" % language % ".qm");
    qApp->installTranslator(d->frontEndTranslator);
}

#include "frontend.moc"
