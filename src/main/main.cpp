/** This file is part of Qt Media Hub**

Copyright (c) 2012 Nokia Corporation and/or its subsidiary(-ies).*
All rights reserved.

Contact:  Nokia Corporation qmh-development@qt-project.org

You may use this file under the terms of the BSD license
as follows:

Redistribution and use in source and binary forms, with or
without modification, are permitted provided that the following
conditions are met:
* Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.

* Neither the name of Nokia Corporation and its Subsidiary(-ies)
nor the names of its contributors may be used to endorse or promote
products derived from this software without specific prior
written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. **/

#include "mainwindow.h"
#include "mediaserver.h"
#include "globalsettings.h"
#include "skin.h"
#include "skinmanager.h"
#include "libraryinfo.h"

#include <QNetworkProxy>
#include <QNetworkConfigurationManager>
#include <QNetworkSession>

#include <QGuiApplication>

static QNetworkSession *g_networkSession = 0;

static void setupNetwork(GlobalSettings *settings)
{
    QNetworkProxy proxy;
    if (settings->isEnabled(GlobalSettings::Proxy)) {
        QString proxyHost(settings->value(GlobalSettings::ProxyHost).toString());
        int proxyPort = settings->value(GlobalSettings::ProxyPort).toInt();
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

static void logMessageHandler(QtMsgType type,
                              const QMessageLogContext &,
                              const QString &msg)
{
    QString logPath(LibraryInfo::logPath() + "/qmh-log");

    switch(type)
    {
        case QtDebugMsg:
        case QtWarningMsg:
            logPath += "-debug.log";
            break;
        default:
            logPath += "-error.log";
            break;
    }

    QFile logFile(logPath);
    logFile.open(QIODevice::WriteOnly);
    logFile.write(msg.toLatin1());
    logFile.close();
}

int main(int argc, char** argv)
{
    QGuiApplication app(argc, argv);
    app.setApplicationName("sasquatch");
    app.setOrganizationName("MediaTrolls");
    app.setOrganizationDomain("sasquatch.com");

    GlobalSettings *settings = new GlobalSettings(&app);

    if (app.arguments().contains("--help") || app.arguments().contains("-help") || app.arguments().contains("-h")) {
        printf("Usage: sasquatch [-option value] [-option=value]\n"
               "\n"
               "Options (default):\n");

        for (int i = 0; i < GlobalSettings::OptionLength; ++i) {
            printf("  -%-20s %s \t (%s)\n",
                   qPrintable(settings->name((GlobalSettings::Option)i)),
                   qPrintable(settings->doc((GlobalSettings::Option)i)),
                   qPrintable(settings->value((GlobalSettings::Option)i).toString()));
        }

        printf("\n");

        // try to print skin specific settings
        settings->parseArguments(app.arguments());

        SkinManager *skinManager = new SkinManager(settings);
        if (skinManager->skins().contains(settings->value(GlobalSettings::Skin).toString())) {
            Skin *skin = skinManager->skins().value(settings->value(GlobalSettings::Skin).toString());
            if (!skin->parseManifest())
                return 1;

            printf("\n"
                   "Skin '%s' Options (default):\n", qPrintable(skin->name()));

            Settings *skinSettings = skin->settings();
            foreach (const QString &key, skinSettings->keys()) {
                printf("  -%-20s %s \t (%s)\n",
                       qPrintable(key),
                       qPrintable(skinSettings->doc(key)),
                       qPrintable(skinSettings->value(key).toString()));
            }
        }

        printf("\n");

        return 0;
    }

    // settings store order, commandline arguments rule
    settings->loadConfigFile();
    settings->parseArguments(app.arguments());

    setupNetwork(settings);

    bool redirectDebugging = settings->isEnabled(GlobalSettings::RedirectDebugOutput);
    if (redirectDebugging) qInstallMessageHandler(logMessageHandler);

    MainWindow *mainWindow = 0;
    MediaServer *mediaServer = 0;

    if (!settings->isEnabled(GlobalSettings::Headless)) {
        mainWindow = new MainWindow(settings);
        mainWindow->setSkin(settings->value(GlobalSettings::Skin).toString());
        mainWindow->show();
#if defined Q_OS_MAC
        if (settings->isEnabled(GlobalSettings::UnifiedTimer)) QUnifiedTimer::instance(true)->setConsistentTiming(true);
#endif
    } else {
        mediaServer = new MediaServer(settings);
    }

    int ret = app.exec();

    g_networkSession->close();

    delete mainWindow;
    delete mediaServer;

    return ret;
}
