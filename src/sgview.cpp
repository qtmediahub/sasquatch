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

#include "declarativeview.h"

#include <QQuickItem>
#include <QQmlEngine>
#include <QDebug>

#include "globalsettings.h"

#include "metrics.h"

DeclarativeView::DeclarativeView(GlobalSettings *settings, QWindow *parent)
    : QQuickView(parent),
      m_settings(settings),
      m_glViewport(false),
      m_frameCount(0),
      m_timeSigma(0),
      m_fps(-1)
{
    setResizeMode(QQuickView::SizeRootObjectToView);

    //FIXME: private API does not belong
    m_drivenFPS = false;
    m_overlayMode = false;

    connect(this, SIGNAL(statusChanged(QQuickView::Status)), this, SLOT(handleStatusChanged(QQuickView::Status)));
}

void DeclarativeView::setSource(const QUrl &url)
{
    m_url = url;
    QMetaObject::invokeMethod(this, "handleSourceChanged", Qt::QueuedConnection);
}

void DeclarativeView::handleSourceChanged()
{
    QQuickView::setSource(m_url);
}

void DeclarativeView::handleStatusChanged(QQuickView::Status status)
{
    //Dodgy work around for gnome focus issues?
    if (status == QQuickView::Ready) {
//        requestActivateWindow();
    }
}

QObject *DeclarativeView::focusItem() const
{
    return static_cast<QObject*>(activeFocusItem());
}

int DeclarativeView::fps() const
{
    return m_fps;
}

void DeclarativeView::addImportPath(const QString &path)
{
    engine()->addImportPath(path);
}
