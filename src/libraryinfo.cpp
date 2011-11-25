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
#include "globalsettings.h"

#include <QtCore>
#include <QtGui>

#ifdef SCENEGRAPH
#include <QtWidgets>
#endif

static QStringList standardResourcePaths(GlobalSettings *settings, const GlobalSettings::Option option, const QString &suffix)
{
    static const QString platformBinOffset
        #ifdef Q_OS_MAC
            = "/../../../";
        #else
            = "";
        #endif

    // The order of the added paths is relevant!
    QStringList paths;

    // submodule repo
    paths <<  QCoreApplication::applicationDirPath() % QString::fromLatin1("/../../") % platformBinOffset % suffix % "/";

    // allows changing resource paths with -skinsPath on runtime
    const QString settingsPath = settings->value(option).toString();
    if (!settingsPath.isEmpty())
        paths << QDir(settingsPath).absolutePath();

    // allows changing resource paths with exporting with QMH_SKINS_PATH on runtime
    const QByteArray envPath = qgetenv(QString("QMH_" % suffix.toUpper() % "_PATH").toLatin1());
    if (!envPath.isEmpty())
        paths << QDir(envPath).absolutePath();

    paths << QMH_PROJECTROOT % QString::fromLatin1("/share/qtmediahub/") % suffix % "/";
    paths << QDir::homePath() % QString::fromLatin1("/.qtmediahub/") % suffix % "/";
    paths << QMH_PREFIX % QString::fromLatin1("/share/qtmediahub/") % suffix % "/";

    return paths;
}

QStringList LibraryInfo::translationPaths(GlobalSettings *settings)
{
    return standardResourcePaths(settings, GlobalSettings::TranslationsPath, "translations");
}

QStringList LibraryInfo::resourcePaths(GlobalSettings *settings)
{
    return standardResourcePaths(settings, GlobalSettings::ResourcesPath, "resources");
}

QStringList LibraryInfo::skinPaths(GlobalSettings *settings)
{
    return standardResourcePaths(settings, GlobalSettings::SkinsPath, "skins");
}

QStringList LibraryInfo::applicationPaths(GlobalSettings *settings)
{
    return standardResourcePaths(settings, GlobalSettings::AppsPath, "apps");
}

QStringList LibraryInfo::keyboardMapPaths(GlobalSettings *settings)
{
    return standardResourcePaths(settings, GlobalSettings::KeymapsPath, "keymaps");
}

QStringList LibraryInfo::qmlImportPaths(GlobalSettings *settings)
{
    return standardResourcePaths(settings, GlobalSettings::ImportsPath, "imports");
}

QStringList LibraryInfo::pluginPaths()
{
    QStringList ret;

    ret << QMH_PROJECTROOT % QString::fromLatin1("/lib/qtmediahub/");
    ret << QMH_PREFIX % QString::fromLatin1("/lib/qtmediahub/");

    return ret;
}

QString LibraryInfo::thumbnailPath(GlobalSettings *settings)
{
    const QString path = settings->value(GlobalSettings::ThumbnailPath).toString();
    if (!path.isEmpty())
        return path;

    return QDir::homePath() % "/.thumbnails/" % QApplication::applicationName() % "/";
}

QString LibraryInfo::databaseFilePath()
{
    return LibraryInfo::dataPath() % "/media.db";
}
