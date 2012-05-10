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

#include "trackpad.h"
#include <QtGui>
#ifndef QT5
#include <QtDeclarative>
#endif

Trackpad::Trackpad(QObject *p)
    : QObject(p), parent(p)
{
}

Trackpad::~Trackpad()
{
}

void Trackpad::setRecipient(QObject *recipient)
{
#ifdef QT5
    Q_UNUSED(recipient)
#else
    QDeclarativeView *potentialDeclarativeView = qobject_cast<QDeclarativeView*>(recipient);
    if (potentialDeclarativeView)
    {
        m_recipientContext = QWeakPointer<QDeclarativeContext>(potentialDeclarativeView->rootContext());
        m_recipient = QWeakPointer<QWidget>(potentialDeclarativeView->viewport());
    }
#endif
}

void Trackpad::setEnabled(bool e)
{
#ifdef QT5
    Q_UNUSED(e)
#else
    if(m_recipientContext.isNull()) {
        qWarning("Trying to use Declarative specific functionality outside of Declarative");
        return;
    }
    QDeclarativeExpression expression(m_recipientContext.data(), 0, QString("runtime.cursor.enableCursor(%1)").arg(e));
    expression.evaluate();
    if (expression.hasError())
        qWarning() << "Failed to enable/disable cursor";
#endif
}

void Trackpad::moveBy(qlonglong x, qlonglong y)
{
#ifdef QT5
    Q_UNUSED(x)
    Q_UNUSED(y)
#else
    if(m_recipientContext.isNull()) {
        qWarning("Trying to use Declarative specific functionality outside of Declarative");
        return;
    }

    QDeclarativeExpression expression(m_recipientContext.data(), 0, QString("runtime.cursor.moveBy(%1,%2)").arg(x).arg(y));
    expression.evaluate();
    if (expression.hasError())
        qWarning() << "Failed to enable/disable cursor";
#endif
}

void Trackpad::click()
{
#ifndef QT5
    if(m_recipient.isNull()) {
        qFatal("No recipient has been specified for mouse events");
        return;
    }
    QPoint globalPos = QCursor::pos();
    QPoint localPos = m_recipient.data()->mapFromGlobal(globalPos);
    QMouseEvent mousePress(QEvent::MouseButtonPress, localPos, globalPos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    qApp->sendEvent(m_recipient.data(), &mousePress);
    QMouseEvent mouseRelease(QEvent::MouseButtonRelease, localPos, globalPos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    qApp->sendEvent(m_recipient.data(), &mouseRelease);
#endif
}
