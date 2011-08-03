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

#include "declarativeview.h"

DeclarativeView::DeclarativeView(QWidget *parent)
    : QDeclarativeView(parent),
      m_frameCount(0)
{
    startTimer(1000);
    connect(this, SIGNAL(statusChanged(QDeclarativeView::Status)), this, SLOT(handleStatusChanged(QDeclarativeView::Status)));
}

void DeclarativeView::setSource(const QUrl &url)
{
    m_url = url;
    QMetaObject::invokeMethod(this, "handleSourceChanged", Qt::QueuedConnection);
}

void DeclarativeView::paintEvent(QPaintEvent *event)
{
    m_frameTimer.restart();
    QGraphicsView::paintEvent(event);
    m_timeSigma += m_frameTimer.elapsed();
    ++m_frameCount;
}

void DeclarativeView::timerEvent(QTimerEvent *event)
{
    if (m_timeSigma) {
        int cap = 1000*m_frameCount/m_timeSigma;
        m_timeSigma = m_frameCount = 0;
        emit fpsCap(cap);
    }
    QDeclarativeView::timerEvent(event);
}

void DeclarativeView::handleSourceChanged()
{
    QDeclarativeView::setSource(m_url);
}

void DeclarativeView::handleStatusChanged(QDeclarativeView::Status status)
{
    if (status == QDeclarativeView::Ready) {
        activateWindow();
    }
}

