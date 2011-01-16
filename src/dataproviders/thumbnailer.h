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

#ifndef THUMBNAILER_H
#define THUMBNAILER_H

#include <QObject>
#include <QRunnable>
#include <QImageReader>
#include <QModelIndex>
#include <QFileInfo>

#define THUMBNAILSIZE 200

class Thumbnailer : public QObject
{
    Q_OBJECT
public:
    explicit Thumbnailer(QObject *parent = 0);
    static Thumbnailer *getInstance();

    bool canThumbnail(QFileInfo info);
    QString thumbnail(QString filePath, QModelIndex modelIndex);
    void threadFinished(QModelIndex modelIndex);

signals:
    void creationFinished(QModelIndex modelIndex);

public slots:

private:
    static Thumbnailer *mInstance;
};

class ThumbnailerThread : public QRunnable
{

public:
    ThumbnailerThread(QString filePath, QString thumbnailPath, QModelIndex modelIndex) {
        mFilePath = filePath;
        mThumbnailPath = thumbnailPath;
        mModelIndex = modelIndex;
    }

    void run() {
        QImageReader reader;
        reader.setFileName(mFilePath);
        reader.setQuality(100);

        if (reader.canRead()) {
            QSize orig = reader.size();

            if (orig.width() > THUMBNAILSIZE || orig.height() > THUMBNAILSIZE ) {
                if (orig.width() < orig.height())
                    reader.setScaledSize(QSize(orig.width()/(qreal)orig.height()*THUMBNAILSIZE, THUMBNAILSIZE));
                else
                    reader.setScaledSize(QSize(THUMBNAILSIZE, orig.height()/(qreal)orig.width()*THUMBNAILSIZE));
            }

            QImage newPix = reader.read();
            if (!newPix.isNull())
                newPix.save(mThumbnailPath);
        }

        Thumbnailer::getInstance()->threadFinished(mModelIndex);
    }

    QString mFilePath;
    QString mThumbnailPath;
    QModelIndex mModelIndex;
};

#endif // THUMBNAILER_H
