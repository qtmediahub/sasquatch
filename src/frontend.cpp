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

#include <QDebug>
#include <QFileInfo>

#include <QDesktopWidget>
#include <QDeclarativeView>
#include <QDeclarativeEngine>
#include <QDeclarativeComponent>
#include <QDeclarativeContext>
#include <QApplication>
#include <QTimer>

#ifdef GLVIEWPORT
#include <QGLWidget>
#endif

#include "qmlextensions/customcursor.h"

struct FrontEndPrivate : public QObject
{
    Q_OBJECT
public:
    FrontEndPrivate();
    QWidget *widget;
    const QString platformPathOffset;
    const QString skinPath;
    const QString resourcePath;
};

FrontEndPrivate::FrontEndPrivate()
    : widget(0),
#ifdef Q_OS_MAC
      platformPathOffset("/../../../.."),
#endif
      skinPath(QCoreApplication::applicationDirPath() + platformPathOffset + "/skins"),
      resourcePath(QCoreApplication::applicationDirPath() + platformPathOffset + "/resources")
{ /**/ }

FrontEnd::FrontEnd(QObject *p)
    : QObject(p),
      d(new FrontEndPrivate())
{
}

FrontEnd::~FrontEnd()
{
    delete d;
    d = 0;
}

QWidget* FrontEnd::loadFrontEnd(const QUrl &url)
{
    bool visible = false;
    if(d->widget)
        visible = d->widget->isVisible();

    delete d->widget;

    QUrl targetUrl;

    if(url.isEmpty() || !url.isValid())
        targetUrl = QUrl::fromLocalFile(d->skinPath + "/confluence/720p/Confluence.qml");
    else
        targetUrl = url;

    if(targetUrl.path().right(3) == "qml")
    {
        QDesktopWidget *desktop = qApp->desktop();

        //FIXME: system settings
        bool scalingAllowed = true;

        QDeclarativeView *widget= new QDeclarativeView();
        qmlRegisterType<CustomCursor>("CustomCursor", 1, 0, "CustomCursor");

        if(scalingAllowed) {
            widget->scale(qreal(desktop->width())/1280, qreal(desktop->height())/720);
            widget->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform | QPainter::TextAntialiasing);
        }

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
        widget->rootContext()->setContextProperty("resourcePath", d->resourcePath);
        widget->rootContext()->setContextProperty("skinPath", d->skinPath);

        widget->setSource(targetUrl);

        d->widget = widget;
    }

    if(visible)
        d->widget->showFullScreen();

    return d->widget;
}

void FrontEnd::show()
{
    if(!d->widget)
        loadFrontEnd(QUrl());
    d->widget->showFullScreen();
}

#include "frontend.moc"
