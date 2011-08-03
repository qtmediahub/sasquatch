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

#include "mainwindow.h"
#include "qmh-config.h"
#include "qmh-util.h"

#include <QGraphicsView>
#include <QDeclarativeView>
#include <QShortcut>

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent),
      m_centralWidget(0)
{
    setOrientation(Config::value("orientation", ScreenOrientationAuto));

    Utils::optimizeWidgetAttributes(this, true);

    new QShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::META + Qt::ALT + Qt::Key_Backspace), this, SLOT(resetUI()));
    new QShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::META + Qt::ALT + Qt::Key_Down), this, SIGNAL(shrink()));
    new QShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::META + Qt::ALT + Qt::Key_Up), this, SIGNAL(grow()));
    new QShortcut(QKeySequence(Qt::ALT + Qt::Key_Return), this, SIGNAL(toggleFullScreen()));

    m_resizeSettleTimer.setSingleShot(true);

    connect(&m_resizeSettleTimer, SIGNAL(timeout()), this, SLOT(handleResize()));
}

MainWindow::~MainWindow()
{
}

QWidget *MainWindow::centralWidget() const
{
    return m_centralWidget;
}

void MainWindow::setCentralWidget(QWidget *centralWidget)
{
    m_centralWidget = centralWidget;
    m_centralWidget->setParent(this);
    Utils::optimizeWidgetAttributes(m_centralWidget, true);

    if (QAbstractScrollArea *scrollArea = qobject_cast<QAbstractScrollArea*>(m_centralWidget)) {
        //Does not appear to work here
        //Not sure why
        Utils::optimizeWidgetAttributes(scrollArea->viewport(), true);
    }
}

void MainWindow::resizeEvent(QResizeEvent *e)
{
    Q_UNUSED(e);
    static int staggerResizingDelay = Config::value("resize-delay", 25);
    m_resizeSettleTimer.start(staggerResizingDelay);
}

void MainWindow::setOrientation(ScreenOrientation orientation)
{
#if defined(Q_OS_SYMBIAN)
    // If the version of Qt on the device is < 4.7.2, that attribute won't work
    if (orientation != ScreenOrientationAuto) {
        const QStringList v = QString::fromAscii(qVersion()).split(QLatin1Char('.'));
        if (v.count() == 3 && (v.at(0).toInt() << 16 | v.at(1).toInt() << 8 | v.at(2).toInt()) < 0x040702) {
            qWarning("Screen orientation locking only supported with Qt 4.7.2 and above");
            return;
        }
    }
#endif // Q_OS_SYMBIAN

    Qt::WidgetAttribute attribute;
    switch (orientation) {
#if QT_VERSION < 0x040702
    // Qt < 4.7.2 does not yet have the Qt::WA_*Orientation attributes
    case ScreenOrientationLockPortrait:
        attribute = static_cast<Qt::WidgetAttribute>(128);
        break;
    case ScreenOrientationLockLandscape:
        attribute = static_cast<Qt::WidgetAttribute>(129);
        break;
    default:
    case ScreenOrientationAuto:
        attribute = static_cast<Qt::WidgetAttribute>(130);
        break;
#else // QT_VERSION < 0x040702
    case ScreenOrientationLockPortrait:
        attribute = Qt::WA_LockPortraitOrientation;
        break;
    case ScreenOrientationLockLandscape:
        attribute = Qt::WA_LockLandscapeOrientation;
        break;
    default:
    case ScreenOrientationAuto:
        attribute = Qt::WA_AutoOrientation;
        break;
#endif // QT_VERSION < 0x040702
    };
    setAttribute(attribute, true);
}

void MainWindow::handleResize()
{
    if (m_centralWidget)
        m_centralWidget->setFixedSize(size());
}

void MainWindow::resetUI()
{
    if (QDeclarativeView *declarativeWidget = qobject_cast<QDeclarativeView*>(m_centralWidget)) {
        QObject *coreObject = declarativeWidget->rootObject();
        QMetaObject::invokeMethod(coreObject, "reset");
    }
}
