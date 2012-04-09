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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#ifdef QT5
#include <QWindow>
#else
#include <QWidget>
#endif

#include <QTimer>

class SkinRuntime;
class SkinManager;
class Skin;
class GlobalSettings;

#if defined(QMH_LIB)
#  define EXPORT_SYM Q_DECL_EXPORT
#else
#  define EXPORT_SYM Q_DECL_IMPORT
#endif

#ifdef QT5
class MainWindow : public QWindow
#else
class MainWindow : public QWidget
#endif
{
    Q_OBJECT
public:
    enum ScreenOrientation {
        ScreenOrientationLockPortrait,
        ScreenOrientationLockLandscape,
        ScreenOrientationAuto
    };
#ifdef QT5
    EXPORT_SYM MainWindow(GlobalSettings *m_settings, QWindow *parent = 0);
#else
    EXPORT_SYM MainWindow(GlobalSettings *m_settings, QWidget *parent = 0);
#endif
    ~MainWindow();

#ifdef QT5
// TODO: rename to centralWindow
    void setCentralWidget(QWindow *cw);
    QWindow *centralWidget() const;
#else
    void setCentralWidget(QWidget *cw);
    QWidget *centralWidget() const;
#endif

public slots:
    // ## These are a bit evil, since they shadow QWidget
    EXPORT_SYM void show();
    void showFullScreen();
    void showNormal();
    EXPORT_SYM bool setSkin(const QString &name);
    bool setSkin(Skin *skin);

signals:
    void resetUI();
    void inputIdle();
    void inputActive();

protected:
    void resizeEvent(QResizeEvent *e);
    bool eventFilter(QObject *obj, QEvent *event);

private slots:
    void toggleFullScreen();
    void increaseHeight();
    void decreaseHeight();
    void increaseWidth();
    void decreaseWidth();
    void moveLeft();
    void moveRight();
    void moveUp();
    void moveDown();

    void setOrientation(ScreenOrientation orientation);
    void handleResize();
    void selectSkin();

private:
    QTimer m_resizeSettleTimer;
    QTimer m_inputIdleTimer;
    SkinManager *m_skinManager;
    SkinRuntime *m_skinRuntime;
#ifdef QT5
    QWindow *m_centralWidget;
#else
    QWidget *m_centralWidget;
#endif
    bool m_overscanWorkAround;
    bool m_attemptingFullScreen;
    GlobalSettings *m_settings;
};

#endif // MAINWINDOW_H
