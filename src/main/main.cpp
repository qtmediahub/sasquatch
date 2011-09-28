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
#include "mediaserver.h"
#include "qmh-config.h"
#include "settings.h"

#include <QApplication>
#include <QNetworkProxy>
#include <QNetworkConfigurationManager>
#include <QNetworkSession>

#ifdef SCENEGRAPH
#include <QApplication>
#else
#include "qtsingleapplication.h"
#endif

static QNetworkSession *g_networkSession = 0;

static void setupNetwork()
{
    QNetworkProxy proxy;
    if (Settings::isEnabled(Settings::Proxy)) {
        QString proxyHost(Settings::value(Settings::ProxyHost).toString());
        int proxyPort = Settings::value(Settings::ProxyPort).toInt();
        proxy.setType(QNetworkProxy::HttpProxy);
        proxy.setHostName(proxyHost);
        proxy.setPort(proxyPort);
        QNetworkProxy::setApplicationProxy(proxy);
        qWarning() << "Using proxy host" << proxyHost << "on port" << proxyPort;
    }

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
}

int main(int argc, char** argv)
{
#ifdef SCENEGRAPH
    QApplication app(argc, argv);
#else

    bool overrideGraphicsSystem = false;
    for(int i = 0; i < argc; ++i) {
        if (qstrcmp(argv[i], "-graphicssystem") == 0) {
            overrideGraphicsSystem = true;
            break;
        }
    }

    if (!overrideGraphicsSystem)
        QApplication::setGraphicsSystem("raster");

    QtSingleApplication app(argc, argv);
#endif
    app.setApplicationName("qtmediahub");
    app.setOrganizationName("MediaTrolls");
    app.setOrganizationDomain("qtmediahub.com");

    if (app.arguments().contains("--help") || app.arguments().contains("-help") || app.arguments().contains("-h")) {
        Settings::instance()->init(QStringList());
        printf("Usage: qtmediahub [-option value] [-option=value]\n"
               "\n"
               "Options (default):\n");

        for (int i = 0; i < Settings::OptionLength; ++i) {
            printf("  -%-20s %s \t (%s)\n",
                   qPrintable(Settings::instance()->name((Settings::Option)i)),
                   qPrintable(Settings::instance()->doc((Settings::Option)i)),
                   qPrintable(Settings::instance()->value((Settings::Option)i).toString()));
        }
        return 0;
    }

    Settings::instance()->init(app.arguments());

    setupNetwork();

#ifndef SCENEGRAPH
    bool primarySession = !app.isRunning();
    if (!(Settings::isEnabled(Settings::MultiInstance) || primarySession)) {
        qWarning() << app.applicationName() << "is already running, aborting";
        return false;
    }
#endif

    Config::init(argc, argv);

    MainWindow *mainWindow = 0;
    MediaServer *mediaServer = 0;

    if (!Settings::isEnabled(Settings::Headless)) {
        mainWindow = new MainWindow;
        mainWindow->setSkin(Settings::value(Settings::Skin).toString());
        mainWindow->show();
    } else {
        mediaServer = new MediaServer;
    }

    int ret = app.exec();
    
    g_networkSession->close();

    delete mainWindow;
    delete mediaServer;

    return ret;
}
