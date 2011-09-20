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
#include "qmh-config.h"
#include "skinmanager.h"
#include "skinselector.h"
#include "skinruntime.h"

#include <QGraphicsView>
#include <QShortcut>

#ifdef SCENEGRAPH
#include <QtWidgets>
#else
#include <QDeclarativeView>
#endif

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent),
      m_centralWidget(0),
      m_defaultGeometry(0, 0, 1080, 720),
      m_overscanWorkAround(Config::isEnabled("overscan", false)),
      m_attemptingFullScreen(Config::isEnabled("fullscreen", true))
{
    const bool isOverlay = Config::isEnabled("overlay-mode", false);
    setAttribute(isOverlay ? Qt::WA_TranslucentBackground : Qt::WA_NoSystemBackground);

    m_skinRuntime = new SkinRuntime(this);

    setOrientation(Config::value("orientation", ScreenOrientationAuto));

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

    m_inputIdleTimer.setInterval(Config::value("idle-timeout", 120)*1000);
    m_inputIdleTimer.setSingleShot(true);
    m_inputIdleTimer.start();
    connect(&m_inputIdleTimer, SIGNAL(timeout()), this, SIGNAL(inputIdle()));

    QList<QAction*> actions;
    QAction *selectSkinAction = new QAction(tr("Select skin"), this);
    QAction *quitAction = new QAction(tr("Quit"), this);
    connect(selectSkinAction, SIGNAL(triggered()), this, SLOT(selectSkin()));
    connect(quitAction, SIGNAL(triggered()), this, SLOT(close()));
    actions.append(selectSkinAction);
    actions.append(quitAction);

    if (Config::isEnabled("systray", true)) {
        QSystemTrayIcon *systray = new QSystemTrayIcon(QIcon(":/images/petite-ganesh-22x22.jpg"), this);
        systray->setVisible(true);
        QMenu *contextMenu = new QMenu;
        contextMenu->addActions(actions);
        systray->setContextMenu(contextMenu);
    }

	m_skinManager = new SkinManager(this);
}

MainWindow::~MainWindow()
{
    Config::setEnabled("overscan", m_overscanWorkAround);
    Config::setValue("desktop-id", qApp->desktop()->screenNumber(this));
    Config::setEnabled("fullscreen", m_attemptingFullScreen);
    if (!m_attemptingFullScreen)
        Config::setValue("window-geometry", geometry());
    else if (m_overscanWorkAround)
        Config::setValue("overscan-geometry", geometry());
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
    static int staggerResizingDelay = Config::value("resize-delay", 25);
    m_resizeSettleTimer.start(staggerResizingDelay);
}

void MainWindow::setOrientation(ScreenOrientation orientation)
{
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
}

void MainWindow::handleResize()
{
    if (m_centralWidget)
        m_centralWidget->setFixedSize(size());
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
        Config::setEnabled("overscan", false);
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
        Config::setEnabled("overscan", false);
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
        Config::setEnabled("overscan");
        showFullScreen();
    }
    setGeometry(geometry().adjusted(0,1,0,-1));
}

void MainWindow::decreaseWidth()
{
    if (!m_attemptingFullScreen)
        return;

    if (!m_overscanWorkAround) {
        Config::setEnabled("overscan");
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
    if (m_attemptingFullScreen) {
        Config::setValue("overscan-geometry", geometry());
        showNormal();
    } else {
        Config::setValue("window-geometry", geometry());
        showFullScreen();
    }
}

void MainWindow::showFullScreen()
{
    m_attemptingFullScreen = true;
    m_overscanWorkAround = Config::isEnabled("overscan", false);

    if (m_overscanWorkAround) {
        QRect geometry = Config::value("overscan-geometry", m_defaultGeometry);
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
    setGeometry(Config::value("window-geometry", m_defaultGeometry));
    QWidget::showNormal();

    activateWindow();
}

void MainWindow::show()
{
    if (m_attemptingFullScreen) {
        showFullScreen();
    } else {
        showNormal();
    }
}

void MainWindow::selectSkin()
{
    SkinSelector *skinSelector = new SkinSelector(m_skinManager, this);
    skinSelector->setAttribute(Qt::WA_DeleteOnClose);
    connect(skinSelector, SIGNAL(skinSelected(Skin *)), this, SLOT(setSkin(Skin *)));
    skinSelector->exec();
}

bool MainWindow::setSkin(const QString &name)
{
    QHash<QString, Skin *> skins = m_skinManager->skins();
    Skin *newSkin = skins.value(name);
    if (!newSkin && skins.size() > 0) {
        newSkin = skins.value(Config::value("default-skin", "confluence").toString());
    }

    if (!newSkin && skins.size() > 0) {
        newSkin = skins.value(skins.keys().at(0));
    }

    if (!newSkin) {
        qDebug() << "Failed to set skin:" << name;
        return false;
    }
    setSkin(newSkin);
    return true;
}

bool MainWindow::setSkin(Skin *newSkin)
{
    QObject *skinWidget = m_skinRuntime->create(newSkin, this);
    if (!skinWidget) {
        qDebug() << "Failed to load skin:" << newSkin->name();
        return false;
    }

    QWidget *widget = qobject_cast<QWidget*>(skinWidget);
    if (widget) {
        setCentralWidget(widget);
    }
    return true;
}

