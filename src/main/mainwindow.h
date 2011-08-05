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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include <QTimer>

class Frontend;
class Skin;

class MainWindow : public QWidget
{
    Q_OBJECT
public:
    enum ScreenOrientation {
        ScreenOrientationLockPortrait,
        ScreenOrientationLockLandscape,
        ScreenOrientationAuto
    };
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void setCentralWidget(QWidget *cw);
    QWidget *centralWidget() const;

public slots:
    // ## These are a bit evil, since they shadow QWidget
    void show();
    void showFullScreen();
    void showNormal();
    bool setSkin(const QString &name);
    bool setSkin(Skin*skin);

signals:
    void resetUI();
    void inputIdle();
    void inputActive();

protected:
    void resizeEvent(QResizeEvent *e);
    bool eventFilter(QObject *obj, QEvent *event);

private slots:
    void toggleFullScreen();
    void grow();
    void shrink();
    void setOrientation(ScreenOrientation orientation);
    void handleResize();
    void selectSkin();

private:
    QTimer m_resizeSettleTimer;
    QTimer m_inputIdleTimer;
    Frontend *m_frontend;
    QWidget *m_centralWidget;
    const QRect m_defaultGeometry;
    bool m_overscanWorkAround;
    bool m_attemptingFullScreen;
};

#endif // MAINWINDOW_H
