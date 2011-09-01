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

#include "libraryinfo.h"
#include "qmh-config.h"

#include <QDesktopServices>

static QStringList standardResourcePaths(const QString &suffix)
{
    QString basePath = LibraryInfo::basePath();
    QStringList paths;

    // TODO check below two paths if still valid maybe basePath is wrong as the default value
    paths <<  basePath % QString::fromLatin1("/") % suffix; // unified repo
#ifdef Q_OS_MAC
    paths <<  basePath % QString::fromLatin1("/../../../") % suffix; // submodule repo
#else
    paths <<  basePath % QString::fromLatin1("/../../") % suffix; // submodule repo
#endif

    paths << QMH_INSTALL_PREFIX % QString::fromLatin1("/") % suffix % QString::fromLatin1("/");
    paths << QDir::homePath() % "/.qtmediahub/" % suffix % QString::fromLatin1("/");

    QString configPath = Config::value(suffix % "-path", QString());
    if (!configPath.isEmpty())
        paths << QDir(configPath).absolutePath();

    QString envVar("QMH_" % suffix.toUpper() % "PATH");
    if (!qgetenv(envVar.toLatin1().constData()).isEmpty())
        paths << QDir(qgetenv(envVar.toLatin1().constData())).absolutePath();

    return paths;
}

static QString storageLocation(QDesktopServices::StandardLocation type)
{
    QString location = QDesktopServices::storageLocation(type);
    return location.isEmpty() ? QString("/tmp") : location;
}

QString LibraryInfo::dataPath()
{
    return storageLocation(QDesktopServices::DataLocation);
}

QString LibraryInfo::tempPath()
{
    return storageLocation(QDesktopServices::TempLocation);
}

QString LibraryInfo::logPath()
{
    return storageLocation(QDesktopServices::TempLocation);
}

QString LibraryInfo::basePath()
{
    static QString cachedBasePath;
    if (!cachedBasePath.isEmpty())
        return cachedBasePath;

#ifdef Q_OS_MAC
    QString platformOffset("/../../../");
#else
    QString platformOffset;
#endif

    QString defaultBasePath(QMH_INSTALL_PREFIX);
    if (Config::value("testing", false) || !QDir(defaultBasePath).exists()) {
        qDebug() << "Either testing or uninstalled: running in build dir";
        defaultBasePath = QCoreApplication::applicationDirPath() + platformOffset;
    }
    cachedBasePath = Config::value("base-path",  defaultBasePath);
    return cachedBasePath;
}

QString LibraryInfo::translationPath()
{
    return basePath() % "/translations/";
}

QString LibraryInfo::pluginPath()
{
    return QDir(Config::value("plugins-path", QString(basePath() % "/plugins"))).absolutePath();
}

QString LibraryInfo::resourcePath()
{
    if (!qgetenv("QMH_RESOURCEPATH").isEmpty())
        return QDir(qgetenv("QMH_RESOURCEPATH")).absolutePath();
    else
        return QDir(Config::value("resources-path", QString(basePath() % "/resources"))).absolutePath();
}

QString LibraryInfo::thumbnailPath()
{
    return Config::value("thumbnail-path", QDir::homePath() + "/.thumbnails/" + QApplication::applicationName() + "/");
}

QStringList LibraryInfo::skinPaths()
{
    return standardResourcePaths("skins");
}

QStringList LibraryInfo::applicationPaths()
{
    return standardResourcePaths("apps");
}

QString LibraryInfo::databaseFilePath()
{
    return LibraryInfo::dataPath() + "/media.db";
}

QString LibraryInfo::keyboardMapPath()
{
    return LibraryInfo::basePath() + "/devices/keymaps/";
}

