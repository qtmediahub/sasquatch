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

#ifndef PLAYLIST_H
#define PLAYLIST_H

#include <QAbstractListModel>

#include "../plugins/mediainfo.h"

class PlaylistPrivate;

class Playlist : public QAbstractListModel
{
    Q_OBJECT
public:
    Playlist(QObject *parent = 0);
    ~Playlist();

    enum CustomRoles {
        // Qt::UserRole+1 to 100 are reserved by this model!
        PreviewUrlRole = Qt::UserRole + 1,
        FilePathRole,
        FileNameRole,
        FileUrlRole,
        MediaInfoTypeRole,
        FileSizeRole,
        FileDateTimeRole
    };

    int rowCount(const QModelIndex &parent) const;
    int count() const { return rowCount(QModelIndex()); }
    QVariant data(const QModelIndex &index, int role) const;

signals:

public slots:
    void add(const MediaInfo *info);

private:
    Q_DISABLE_COPY(Playlist)
    PlaylistPrivate *d;
};

#endif // PLAYLIST_H
