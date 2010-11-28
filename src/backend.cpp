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

#include "backend.h"
#include "plugins/qmhplugininterface.h"
#include "plugins/qmhplugin.h"
#include "dataproviders/foldermodel.h"

#include <QDir>
#include <QString>
#include <QPluginLoader>
#include <QCoreApplication>
#include <QVariant>

#include <QDebug>

Backend* Backend::pSelf = 0;

struct BackendPrivate
{
    BackendPrivate()
        :
      #ifdef Q_OS_MAC
          platformOffset("/../../.."),
      #endif
          basePath(QCoreApplication::applicationDirPath() + platformOffset),
          skinPath(basePath + "/skins"),
          pluginPath(basePath + "/plugins"),
          resourcePath(basePath + "/resources") { /* */ }

    QList<QObject*> engines;

    const QString platformOffset;

    const QString basePath;
    const QString skinPath;
    const QString pluginPath;
    const QString resourcePath;
};

Backend::Backend(QObject *parent)
    : QObject(parent),
      d(new BackendPrivate())
{
    // register dataproviders to QML
    qmlRegisterType<FolderModel>("FolderModel", 1, 0, "FolderModel");
    qmlRegisterType<QMHPlugin>("QMHPlugin", 1, 0, "QMHPlugin");

    discoverEngines();
}

void Backend::discoverEngines()
{
    foreach(const QString fileName, QDir(pluginPath()).entryList(QDir::Files)) {
        QString qualifiedFileName(pluginPath() + "/" + fileName);
        QPluginLoader pluginLoader(qualifiedFileName);
        if(pluginLoader.load()
           && qobject_cast<QMHPluginInterface*>(pluginLoader.instance())) {
            QMHPlugin *plugin = new QMHPlugin(qobject_cast<QMHPluginInterface*>(pluginLoader.instance()), this);
            plugin->setParent(this);
            plugin->registerPlugin();
            registerEngine(plugin);
        }
        else
            qDebug() << "Invalid plugin present" << qualifiedFileName << pluginLoader.errorString();
    }
}

QList<QObject*> Backend::engines() const
{
    return d->engines;
}

Backend* Backend::instance()
{
    if(!pSelf) {
        pSelf = new Backend();
    }
    return pSelf;
}

QString Backend::skinPath() const {
    return d->skinPath;
}

QString Backend::pluginPath() const {
    return d->pluginPath;
}

QString Backend::resourcePath() const {
    return d->resourcePath;
}

void Backend::registerEngine(QMHPlugin *engine) {
    d->engines << engine;
    emit enginesChanged();
}

QObject* Backend::engine(const QString &role) {
    foreach(QObject *currentEngine, d->engines )
        if(qobject_cast<QMHPlugin*>(currentEngine)->role() == role)
            return currentEngine;
    qWarning() << "Seeking a non-existant plugin, prepare to die";
    return 0;
}
