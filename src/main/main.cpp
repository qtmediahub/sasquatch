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
#include <QNetworkProxy>
#include "qtsingleapplication.h"

#include "mediaserver.h"
#include "frontend.h"
#include "qmh-config.h"

#if defined(Q_WS_S60) || defined(Q_WS_MAEMO)
static QNetworkSession *g_networkSession = 0;
#endif

static void setupNetwork()
{
    QNetworkProxy proxy;
    if (Config::isEnabled("proxy", false)) {
        QString proxyHost(Config::value("proxy-host", "localhost").toString());
        int proxyPort = Config::value("proxy-port", 8080);
        proxy.setType(QNetworkProxy::HttpProxy);
        proxy.setHostName(proxyHost);
        proxy.setPort(proxyPort);
        QNetworkProxy::setApplicationProxy(proxy);
        qWarning() << "Using proxy host" << proxyHost << "on port" << proxyPort;
    }

#if defined(Q_WS_S60) || defined(Q_WS_MAEMO)
    // Set Internet Access Point
    QNetworkConfigurationManager mgr;
    QList<QNetworkConfiguration> activeConfigs = mgr.allConfigurations();
    if (activeConfigs.count() <= 0)
        return;

    QNetworkConfiguration cfg = activeConfigs.at(0);
    foreach(QNetworkConfiguration config, activeConfigs) {
        if (config.type() == QNetworkConfiguration::UserChoice) {
            cfg = config;
            break;
        }
    }

    g_networkSession = new QNetworkSession(cfg);
    g_networkSession->open();
    g_networkSession->waitForOpened(-1);
#endif
}

int main(int argc, char** argv)
{
    QApplication::setGraphicsSystem("raster");

    QtSingleApplication app(argc, argv);
    app.setApplicationName("qtmediahub");
    app.setOrganizationName("Nokia");
    app.setOrganizationDomain("nokia.com");

    setupNetwork();

    bool primarySession = !app.isRunning();
    if (!(Config::isEnabled("multi-instance", false) || primarySession)) {
        qWarning() << app.applicationName() << "is already running, aborting";
        return false;
    }

    Config::init(argc, argv);

    MediaServer *mediaServer = 0;
    Frontend *frontend = 0;

    if (!Config::isEnabled("headless", qgetenv("DISPLAY").isEmpty())) {
        frontend = new Frontend;
        frontend->setSkin(Config::value("skin", "").toString());
        frontend->show();
    } else {
        mediaServer = new MediaServer;
    }

    int ret = app.exec();
    
#if defined(Q_WS_S60) || defined(Q_WS_MAEMO)
    g_networkSession->close();
#endif

    delete mediaServer;
    delete frontend;

    return ret;
}
