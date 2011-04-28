#ifndef EXIFREADER_H
#define EXIFREADER_H

#include <libexif/exif-data.h>
#include <libexif/exif-utils.h>
#include <libexif/exif-loader.h>

#include <QDateTime>
#include <QString>

class ExifReader 
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
