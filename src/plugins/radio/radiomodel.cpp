/****************************************************************************
 *
 * This file is part of the QtMediaHub project on http://www.gitorious.org.
 *
 * Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).*
 * All rights reserved.
 *
 * Contact:  Nokia Corporation (qt-info@nokia.com)**
 *
 * You may use this file under the terms of the BSD license as follows:
 *
 * "Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
 * * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
 * * Neither the name of Nokia Corporation and its Subsidiary(-ies) nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
 *
 * ****************************************************************************/

#include "radiomodel.h"

#include <QFileInfo>
#include <QProcess>
#include <QXmlStreamReader>

#include "qmh-config.h"

RadioModel::RadioModel(QObject *parent)
    : MediaModel(MediaModel::Radio, parent)
{
    qRegisterMetaType<RadioInfo *>("RadioInfo *");

    QHash<int, QByteArray> roleNames = MediaModel::roleNames();
    roleNames[LengthRole] = "length";
    setRoleNames(roleNames);
}

RadioModel::~RadioModel()
{
}

QVariant RadioModel::data(MediaInfo *mediaInfo, int role) const
{
    RadioInfo *info = static_cast<RadioInfo *>(mediaInfo);


    if (role == Qt::DisplayRole) {
        return info->name;
    } else if (role == LengthRole) {
        return info->length;
    } else {
        return QVariant();
    }
}

static RadioInfo *readPLS(QFileInfo fileInfo)
{
    QSettings settings(fileInfo.filePath(), QSettings::IniFormat);
    QStringList keys = settings.allKeys();

    if (!keys.contains("playlist/NumberOfEntries"))
        return 0;

    if (settings.value("playlist/NumberOfEntries", 0).toInt() <= 0)
        return 0;

    // currently only choose first title
    if (!keys.contains("playlist/File1") || !keys.contains("playlist/Title1") || !keys.contains("playlist/Length1") )
        return 0;

    QString file = settings.value("playlist/File1", "").toString();
    QString title = settings.value("playlist/Title1", "").toString();
    int length = settings.value("playlist/Length1", "").toInt();

    RadioInfo *info = new RadioInfo(fileInfo.filePath());
    info->filePath = file;
    info->name = title;
    info->length = length;

    return info;
}

static RadioInfo *readASX(QFileInfo fileInfo)
{
    QFile file(fileInfo.filePath());
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return 0;

    QString title;
    QString uri;
    QString banner;

    QXmlStreamReader reader(&file);
    while(!reader.atEnd() && !reader.hasError()) {
        QXmlStreamReader::TokenType token = reader.readNext();
        if(token == QXmlStreamReader::StartDocument)
            continue;

        if(token == QXmlStreamReader::StartElement) {
            if(reader.name() == "entry") {
                continue;
            }

            if(reader.name() == "Banner" && reader.attributes().hasAttribute("href")) {
                banner = reader.attributes().value("href").toString();
                continue;
            }

            if(reader.name() == "title") {
                reader.readNext();
                if(reader.tokenType() == QXmlStreamReader::Characters)
                    title = reader.text().toString();
                continue;
            }
            if(reader.name() == "ref" && reader.attributes().hasAttribute("href")) {
                uri = reader.attributes().value("href").toString();
                continue;
            }
        }

        if (title != "" && uri != "")
            break;
    }

    if(reader.hasError() || title == "" || uri == "") {
        reader.clear();
        return 0;
    }

    reader.clear();

    RadioInfo *info = new RadioInfo(fileInfo.filePath());
    info->filePath = uri;
    info->name = title;
    info->thumbnailPath = banner;

    return info;
}

// TDBs
static RadioInfo *readXSPF(QFileInfo fileInfo)
{
    return 0;
}

MediaInfo *RadioModel::readMediaInfo(const QString &filePath)
{
    QFileInfo fileInfo(filePath);
    //FIXME: query supported extensions from underlying media framework
    static QStringList supportedTypes;
    supportedTypes << "pls" << "asx";

    if (!fileInfo.exists() || !supportedTypes.contains(fileInfo.suffix()))
        return 0;

    RadioInfo *info = 0;
    if (fileInfo.suffix() == "pls")
        info = readPLS(fileInfo);
    else if (fileInfo.suffix() == "asx")
        info = readASX(fileInfo);

    if (!info)
        return 0;

    if (info->thumbnailPath == "")
        info->thumbnailPath = themeResourcePath() + "DefaultAudio.png";

    return info;
}

