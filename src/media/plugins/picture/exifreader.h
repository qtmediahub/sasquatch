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

#ifndef EXIFREADER_H
#define EXIFREADER_H

#include <libexif/exif-data.h>
#include <libexif/exif-utils.h>
#include <libexif/exif-loader.h>

#include <QDateTime>
#include <QString>

#include "global.h"

class QMH_EXPORT ExifReader
{
public:
    ExifReader(const QString &file);
    ~ExifReader();

    QString stringTag(ExifTag tag) const;
    QString userComments() const;
    QString imageDescription() const;
    QDateTime creationTime();
    QString cameraModel() const;
    QString cameraMake() const;

    double latitude(bool *ok = 0) const;
    double longitude(bool *ok = 0) const;
    double altitude(bool *ok = 0) const;

    // http://sylvana.net/jpegcrop/exif_orientation.html
    enum Orientation {
        Invalid,
        NoOrientation = 1,
        FlipHorizontal,
        Rotate180,
        FlipVertical,
        Transpose,
        Rotate90,
        Transverse,
        Rotate270
    };
    Orientation orientation() const;

    QString aperture() const;
    QString focalLength() const;
    QString exposureTime() const;
    QString exposureMode() const;
    QString whiteBalance() const;
    QString lightSource() const;
    QString isoSpeed() const;
    QString digitalZoomRatio() const;
    QString flashUsage() const;
    QString colorSpace() const;

private:
    QString stringValue(ExifEntry *entry) const;
    ExifByteOrder m_byteOrder;
    const int m_sizeOfRational;
    ExifData *m_data;
};

#endif // EXIFREADER_H
