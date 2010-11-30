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

#include <QDesktopWidget>
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

#include "qmlextensions/customcursor.h"
#include "config.h"

struct FrontendPrivate : public QObject
{
    Q_OBJECT
public:
    FrontendPrivate();
    ~FrontendPrivate();
    QWidget *widget;
    QTranslator frontEndTranslator;
};

FrontendPrivate::FrontendPrivate()
    : widget(0)
{ /**/ }

FrontendPrivate::~FrontendPrivate()
{
    delete widget;
    widget = 0;
}

Frontend::Frontend(QObject *p)
    : QObject(p),
      d(new FrontendPrivate())
{
}

Frontend::~Frontend()
{
    delete d;
    d = 0;
}

QWidget* Frontend::loadFrontend(const QUrl &url)
{
    bool visible = false, fullScreen = false;
    if(d->widget) {
        visible = d->widget->isVisible();
        fullScreen = d->widget->windowState() & Qt::WindowFullScreen;
    }

    delete d->widget;

    QUrl targetUrl;

    if(url.isEmpty() || !url.isValid())
        targetUrl = Config::value("defaultSkin", QUrl::fromLocalFile(Backend::instance()->skinPath() + "/confluence/720p/Confluence.qml"));
    else
        targetUrl = url;

    //Loading translation is part of loading skin
    //d->translator.load("");
    //qApp->installTranslator(&(d->translator));

    if(targetUrl.path().right(3) == "qml")
    {
        qmlRegisterType<CustomCursor>("CustomCursor", 1, 0, "CustomCursor");

        QDesktopWidget *desktop = qApp->desktop();


        QDeclarativeView *widget= new QDeclarativeView();

        if(Config::isEnabled("scale-ui", false))
            widget->scale(qreal(desktop->width())/1280, qreal(desktop->height())/720);
        if(Config::isEnabled("smooth-scaling", true))
            widget->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform | QPainter::TextAntialiasing);

        widget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        widget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        widget->setFrameStyle(0);
        widget->setOptimizationFlags(QGraphicsView::DontSavePainterState);
        widget->scene()->setItemIndexMethod(QGraphicsScene::NoIndex);

        QObject::connect(widget->engine(), SIGNAL(quit()), qApp, SLOT(quit()));

#ifdef GLVIEWPORT
        widget->setViewport(new QGLWidget());
#endif
#ifdef GL
        widget->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
#else
        widget->setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);
#endif
        Backend::instance()->initialize(widget->engine());

        widget->setSource(targetUrl);

        d->widget = widget;
    }

    if(visible) {
        if (fullScreen)
            d->widget->showFullScreen();
        else
            d->widget->show();
    }

    return d->widget;
}

void Frontend::show()
{
    if(!d->widget)
        loadFrontend(QUrl());
    d->widget->resize(1000, 720);
    d->widget->show();
}

void Frontend::showFullScreen()
{
    if(!d->widget)
        loadFrontend(QUrl());
    d->widget->showFullScreen();
}

#include "frontend.moc"
