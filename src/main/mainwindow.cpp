/****************************************************************************

This file is part of the QtMediaHub project on http://www.qtmediahub.com

Copyright (c) 2011 Nokia Corporation and/or its subsidiary(-ies).*
All rights reserved.

Contact:  Girish Ramakrishnan girish@forwardbias.in
Contact:  Nokia Corporation donald.carr@nokia.com
Contact:  Nokia Corporation johannes.zellner@nokia.com

You may use this file under the terms of the BSD license as follows:

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
* Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
* Neither the name of Nokia Corporation and its Subsidiary(-ies) nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

****************************************************************************/

#include "mainwindow.h"
#include "skinmanager.h"
#include "skinruntime.h"
#include "globalsettings.h"

#include <QtCore>
#include <QtGui>

#ifdef QT5
#include <QQuickView>
#include <QtWidgets>
#else
#include <QDeclarativeView>
#endif

MainWindow::MainWindow(GlobalSettings *settings, QWidget *parent)
    : QWidget(parent),
      m_centralWidget(0),
      m_settings(settings)
{
    m_overscanWorkAround = m_settings->isEnabled(GlobalSettings::Overscan);
    m_attemptingFullScreen = m_settings->isEnabled(GlobalSettings::FullScreen);

    const bool isOverlay = m_settings->isEnabled(GlobalSettings::OverlayMode);
    setAttribute(isOverlay ? Qt::WA_TranslucentBackground : Qt::WA_NoSystemBackground);

    m_skinRuntime = new SkinRuntime(m_settings, this);

    // TODO Orientation needs to be changeable from skin during runtime
    setOrientation(ScreenOrientationAuto);

    new QShortcut(QKeySequence(Qt::ALT + Qt::Key_Backspace), this, SIGNAL(resetUI()));
    new QShortcut(QKeySequence(Qt::ALT + Qt::Key_Down), this, SLOT(decreaseHeight()));
    new QShortcut(QKeySequence(Qt::ALT + Qt::Key_Up), this, SLOT(increaseHeight()));
    new QShortcut(QKeySequence(Qt::ALT + Qt::Key_Right), this, SLOT(decreaseWidth()));
    new QShortcut(QKeySequence(Qt::ALT + Qt::Key_Left), this, SLOT(increaseWidth()));
    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Right), this, SLOT(moveRight()));
    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Left), this, SLOT(moveLeft()));
    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Up), this, SLOT(moveUp()));
    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Down), this, SLOT(moveDown()));

    new QShortcut(QKeySequence(Qt::ALT + Qt::Key_Return), this, SLOT(toggleFullScreen()));

    m_resizeSettleTimer.setSingleShot(true);

    connect(&m_resizeSettleTimer, SIGNAL(timeout()), this, SLOT(handleResize()));

    m_inputIdleTimer.setInterval(m_settings->value(GlobalSettings::IdleTimeout).toInt() * 1000);
    m_inputIdleTimer.setSingleShot(true);
    m_inputIdleTimer.start();
    connect(&m_inputIdleTimer, SIGNAL(timeout()), this, SIGNAL(inputIdle()));

    QList<QAction*> actions;
    QAction *quitAction = new QAction(tr("Quit"), this);
    connect(quitAction, SIGNAL(triggered()), this, SLOT(close()));
    actions.append(quitAction);

    QAction *selectSkinAction = new QAction(tr("Select skin"), this);
    connect(selectSkinAction, SIGNAL(triggered()), this, SLOT(selectSkin()));
    actions.append(selectSkinAction);

    if (m_settings->isEnabled(GlobalSettings::SystemTray)) {
        QSystemTrayIcon *systray = new QSystemTrayIcon(QIcon(":/images/petite-ganesh-22x22.jpg"), this);
        systray->setVisible(true);
        QMenu *contextMenu = new QMenu;
        contextMenu->addActions(actions);
        systray->setContextMenu(contextMenu);
    }

    m_skinManager = new SkinManager(m_settings, this);
}

MainWindow::~MainWindow()
{
    m_settings->setValue(GlobalSettings::Overscan, m_overscanWorkAround);
    m_settings->setValue(GlobalSettings::FullScreen, m_attemptingFullScreen);

    if (!m_attemptingFullScreen)
        m_settings->setValue(GlobalSettings::WindowGeometry, geometry());
    else if (m_overscanWorkAround)
        m_settings->setValue(GlobalSettings::OverscanGeometry, geometry());
}

QWidget *MainWindow::centralWidget() const
{
    return m_centralWidget;
}

void MainWindow::setCentralWidget(QWidget *centralWidget)
{
    if (m_centralWidget) {
        m_centralWidget->hide();
        delete m_centralWidget;
    }
    m_centralWidget = centralWidget;
    m_centralWidget->setParent(this);
    m_centralWidget->setFixedSize(size());
    m_centralWidget->installEventFilter(this);
    if (isVisible())
        m_centralWidget->show();
}

void MainWindow::resizeEvent(QResizeEvent *e)
{
    Q_UNUSED(e);

    static int staggerResizingDelay = m_settings->value(GlobalSettings::ResizeDelay).toInt();
    if (staggerResizingDelay == 0)
        handleResize();
    else
        m_resizeSettleTimer.start(staggerResizingDelay);

    QWidget::resizeEvent(e);
}

void MainWindow::setOrientation(ScreenOrientation orientation)
{
#ifdef QT5
    qWarning("set orientation is not supported, yet");
#else
    Qt::WidgetAttribute attribute;
    switch (orientation) {
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
    };
    setAttribute(attribute, true);
#endif
}

void MainWindow::handleResize()
{
    if (m_centralWidget)
        m_centralWidget->setFixedSize(size());
    qDebug() << "Resizing widget to:" << size();
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress
        || event->type() == QEvent::KeyRelease
        || event->type() == QEvent::MouseMove
        || event->type() == QEvent::MouseButtonPress) {
        if (!m_inputIdleTimer.isActive())
            emit inputActive();
        m_inputIdleTimer.start();
    }

    return QObject::eventFilter(obj, event);
}

void MainWindow::increaseHeight()
{
    if (!m_attemptingFullScreen)
        return;

    const QRect newGeometry = geometry().adjusted(0,-1,0,1);

    const QSize desktopSize = qApp->desktop()->screenGeometry(this).size();
    if ((newGeometry.width() > desktopSize.width())
            || (newGeometry.height() > desktopSize.height())) {
        m_settings->setValue(GlobalSettings::Overscan, false);
        showFullScreen();
    } else {
        setGeometry(newGeometry);
    }
}

void MainWindow::increaseWidth()
{
    if (!m_attemptingFullScreen)
        return;

    const QRect newGeometry = geometry().adjusted(-1,0,1,0);

    const QSize desktopSize = qApp->desktop()->screenGeometry(this).size();
    if ((newGeometry.width() > desktopSize.width())
            || (newGeometry.height() > desktopSize.height())) {
        m_settings->setValue(GlobalSettings::Overscan, false);
        showFullScreen();
    } else {
        setGeometry(newGeometry);
    }

}

void MainWindow::decreaseHeight()
{
    if (!m_attemptingFullScreen)
        return;

    if (!m_overscanWorkAround) {
        m_settings->setValue(GlobalSettings::Overscan, true);
        showFullScreen();
    }
    setGeometry(geometry().adjusted(0,1,0,-1));
}

void MainWindow::decreaseWidth()
{
    if (!m_attemptingFullScreen)
        return;

    if (!m_overscanWorkAround) {
        m_settings->setValue(GlobalSettings::Overscan, true);
        showFullScreen();
    }
    setGeometry(geometry().adjusted(1,0,-1,0));
}

void MainWindow::moveLeft()
{
    QRect g = geometry();
    g.moveLeft(g.left() - 1);
    setGeometry(g);
}

void MainWindow::moveRight()
{
    QRect g = geometry();
    g.moveLeft(g.left() + 1);
    setGeometry(g);
}

void MainWindow::moveDown()
{
    QRect g = geometry();
    g.moveTop(g.top() + 1);
    setGeometry(g);
}

void MainWindow::moveUp()
{
    QRect g = geometry();
    g.moveTop(g.top() - 1);
    setGeometry(g);
}

void MainWindow::toggleFullScreen()
{
    m_settings->setValue(GlobalSettings::OverscanGeometry, geometry());
    if (m_attemptingFullScreen) {
        showNormal();
    } else {
        showFullScreen();
    }
}

void MainWindow::showFullScreen()
{
    m_attemptingFullScreen = true;
    m_overscanWorkAround = m_settings->isEnabled(GlobalSettings::Overscan);

    if (m_overscanWorkAround) {
        QRect oldGeometry = geometry();
        QRect geometry = m_settings->value(GlobalSettings::OverscanGeometry).toRect();
        if (geometry.isNull()) geometry = oldGeometry;

        geometry.moveCenter(qApp->desktop()->availableGeometry().center());

        setGeometry(geometry);
        setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
        setWindowState(Qt::WindowNoState);
        QWidget::show();
    } else {
        QWidget::showFullScreen();
    }

    activateWindow();
}

void MainWindow::showNormal()
{
    m_attemptingFullScreen = m_overscanWorkAround = false;

    setWindowFlags(Qt::Window);
    setGeometry(m_settings->value(GlobalSettings::WindowGeometry).toRect());
    QWidget::showNormal();

    activateWindow();
}

void MainWindow::show()
{
    //FIXME: QML 2 related hackery
#ifndef QT5
    if (m_attemptingFullScreen) {
        showFullScreen();
    } else {
        showNormal();
    }
#endif
}

void MainWindow::selectSkin()
{
    setSkin(0);
}

bool MainWindow::setSkin(const QString &name)
{
    QHash<QString, Skin *> skins = m_skinManager->skins();

    Skin *newSkin = skins.value(name);

    if (newSkin) {
        qWarning() << "Attempting to use:" << name << "skin";
    } else {
        qWarning() << "No skin" << name << "found.";
    }

    return setSkin(newSkin);
}

bool MainWindow::setSkin(Skin *newSkin)
{
    QObject *skinWidget = m_skinRuntime->create(newSkin, this);
    if (!skinWidget) {
        qDebug() << "Failed to load skin or fallback to skin selector";
        return false;
    }

    QWidget *widget = qobject_cast<QWidget*>(skinWidget);
    if (widget) {
        setCentralWidget(widget);
    }
#ifdef QT5
    else {
        //FIXME: We clearly need parity window state management in the long run
        (qobject_cast<QWindow*>(skinWidget))->show();
    }
#endif
    return true;
}

