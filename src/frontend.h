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

#ifndef FRONT_END
#define FRONT_END

#include <QWidget>
#include <QUrl>
#include <QDeclarativeItem>

class FrontendPrivate;

/*Logic abstracting what is handling the rendering and resolution selection*/

class Frontend : public QWidget
{
    Q_OBJECT
    public:
        Frontend(QWidget *p = 0);
        ~Frontend();

        void paintEvent(QPaintEvent *e);
        void resizeEvent(QResizeEvent *e);

        void setSkin(const QString &name);
        void initialize(const QUrl &url);
        void resetLanguage();

        Q_INVOKABLE QObject *focusItem() const;
        Q_INVOKABLE void applyWebViewFocusFix(QDeclarativeItem *item); // See https://bugs.webkit.org/show_bug.cgi?id=51094

    public slots:
        void toggleFullScreen();
        void showFullScreen();
        void showNormal();
        void grow();
        void shrink();

    private:
        FrontendPrivate *d;
};

#endif
