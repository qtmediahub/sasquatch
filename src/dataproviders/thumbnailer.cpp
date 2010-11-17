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

#include "thumbnailer.h"

#include <QCryptographicHash>
#include <QDir>
#include <QThreadPool>

#include "backend.h"

Thumbnailer *Thumbnailer::mInstance = 0;

Thumbnailer::Thumbnailer(QObject *parent) :
    QObject(parent)
{
    mInstance = this;
}

Thumbnailer *Thumbnailer::getInstance()
{
    if (!mInstance)
        new Thumbnailer();

    return mInstance;
}

bool Thumbnailer::canThumbnail(QFileInfo info)
{
    QString s = info.suffix();

    if (s == "jpg" || s == "jpeg" || s == "gif" || s == "png" || s == "bmp")
        return true;
    else
        return false;
}

QString Thumbnailer::thumbnail(QString filePath, QModelIndex modelIndex)
{
    QString ret;
    QString md5 = QCryptographicHash::hash("file://" + filePath.toAscii(), QCryptographicHash::Md5);

    QFileInfo thumbnailInfo(QDir::homePath() + "/.thumbnails/" + THUMBNAILSUBPATH + "/" + md5 + ".png");
    QFileInfo fileInfo(filePath);

    if (thumbnailInfo.exists())
        ret = thumbnailInfo.absoluteFilePath();
    else if (fileInfo.isDir())
        ret = Backend::instance()->skinPath() + "/confluence/3rdparty/skin.confluence" + "/media/DefaultFolder.png";
    else if (canThumbnail(fileInfo)) {
        ThumbnailerThread *thread = new ThumbnailerThread(filePath, thumbnailInfo.absoluteFilePath(), modelIndex);
        QThreadPool::globalInstance()->start(thread);
        ret = Backend::instance()->skinPath() + "/confluence/3rdparty/skin.confluence" + "/media/DefaultPicture.png";
    } else {
        ret = Backend::instance()->skinPath() + "/confluence/3rdparty/skin.confluence" + "/media/DefaultFile.png";
    }

    return ret;
}

void Thumbnailer::threadFinished(QModelIndex modelIndex)
{
    emit creationFinished(modelIndex);
}
