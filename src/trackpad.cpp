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

#include "trackpad.h"
#include "../frontend.h"
#include <QtGui>
#include <QtDeclarative>

Trackpad::Trackpad(QObject *p)
    : QObject(p), parent(p)
{
    setObjectName("trackpad");
}

Trackpad::~Trackpad()
{
}

void Trackpad::setEnabled(bool e)
{
    QDeclarativeView *view = qobject_cast<QDeclarativeView *>(parent);
    QDeclarativeExpression expression(view->rootContext(), 0, QString("cursor.enableCursor(%1)").arg(e));
    expression.evaluate();
    if (expression.hasError())
        qWarning() << "Failed to enable/disable cursor";
}

void Trackpad::moveBy(int x, int y)
{
    QDeclarativeView *view = qobject_cast<QDeclarativeView *>(parent);
    QDeclarativeExpression expression(view->rootContext(), 0, QString("cursor.moveBy(%1,%2)").arg(x).arg(y));
    expression.evaluate();
    if (expression.hasError())
        qWarning() << "Failed to enable/disable cursor";
}

void Trackpad::click()
{
    QPoint globalPos = QCursor::pos();
    QDeclarativeView *view = qobject_cast<QDeclarativeView *>(parent);
    QPoint localPos = view->viewport()->mapFromGlobal(globalPos);
    QMouseEvent mousePress(QEvent::MouseButtonPress, localPos, globalPos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    qApp->sendEvent(view->viewport(), &mousePress);
    QMouseEvent mouseRelease(QEvent::MouseButtonRelease, localPos, globalPos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    qApp->sendEvent(view->viewport(), &mouseRelease);
}
