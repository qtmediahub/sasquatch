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
#ifdef GL
#include <QGLFormat>
#endif
#ifdef QT_SINGLE_APPLICATION
#include "qtsingleapplication.h"
#endif

#include "backend.h"
#include "qmh-config.h"

int main(int argc, char** argv)
{
    QApplication::setGraphicsSystem("raster");

#ifdef GL
    //Purely for experimentation
    QGLFormat format;
    //should suffice
    format.setDepth(false);
//    plan b: russian roulette
//    format.setStencil(false);

#ifdef Q_OS_MAC
    format.setSampleBuffers(true);
#else
    format.setSampleBuffers(false);
#endif //Q_OS_MAC

//    //FIXME: Should be configurable, but Config
//    blocked by instantiation of QApplication
//    //vsync
    format.setSwapInterval(1);
//    //no vsync
//    format.setSwapInterval(0);
//    format.setDoubleBuffer(false);
//    format.setAlpha(false);
//    format.setDirectRendering(false);

    QGLFormat::setDefaultFormat(format);

#ifdef GLGS
    //Only legitimate use is in fullscreen QGraphicsView derived classes!
    //If you use this in conjunction with our traditional QWidget style functionality
    //You are in for a rough ride
    if (Config::isEnabled("use-gl", true))
        QApplication::setGraphicsSystem("opengl");
#endif //GLGS
#endif //GL

#ifdef QT_SINGLE_APPLICATION
    QtSingleApplication app(argc, argv);
#else
    QApplication app(argc, argv);
#endif //QT_SINGLE_APPLICATION

    app.setApplicationName("qtmediahub");
    app.setOrganizationName("Nokia");
    app.setOrganizationDomain("nokia.com");

#ifdef QT_SINGLE_APPLICATION
    if (!Config::isEnabled("multi-instance", false) && app.isRunning()) {
        qWarning() << app.applicationName() << "is already running, aborting";
        return false;
    }
#endif //QT_SINGLE_APPLICATION
    Config::init(argc, argv);

    Backend::instance();

    int ret = app.exec();

    delete Backend::instance();

    return ret;
}
