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

#include <QApplication>
#include <QWidget>
#ifdef GL
#include <QGLFormat>
#endif

#include "backend.h"
#include "frontend.h"
#include "config.h"

int main(int argc, char** argv)
{
    QApplication::setGraphicsSystem("raster");
#ifdef GL
    //Purely for experimentation
    QGLFormat format;
    //should suffice
    format.setDepth(false);
    //plan b: russian roulette
//    format.setStencil(false);
#ifdef Q_OS_MAC
    format.setSampleBuffers(true);
#else
    format.setSampleBuffers(false);
#endif
//    format.setDoubleBuffer(false);
//    format.setAlpha(false);
//    format.setDirectRendering(false);
    QGLFormat::setDefaultFormat(format);
#ifdef GLGS
    QApplication::setGraphicsSystem("opengl");
#endif
#endif
    QApplication app(argc, argv);
    app.setApplicationName("qtmediahub");
    app.setOrganizationName("Nokia");

    Config::init(argc, argv);

    Frontend gui(&app);
    if (Config::isEnabled("start-fullscreen", false)) {
        gui.showFullScreen();
    } else {
        gui.show();
    }

    return app.exec();
}
