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

#include "exifreader.h"

#include <QFile>
#include <QDebug>

ExifReader::ExifReader(const QString &file) :
	m_sizeOfRational(exif_format_get_size(EXIF_FORMAT_RATIONAL))
{
	m_data = exif_data_new_from_file(QFile::encodeName(file).constData());
	if (m_data)
		m_byteOrder = exif_data_get_byte_order(m_data);
}

ExifReader::~ExifReader() 
{
    if (m_data)
	    exif_data_unref(m_data);
}

QString ExifReader::stringTag(ExifTag tag) const 
{
	if (!m_data)
		return QString();

	ExifEntry *entry = exif_data_get_entry(m_data, tag);
	return stringValue(entry);
}

QString ExifReader::userComments() const 
{
	return stringTag(EXIF_TAG_USER_COMMENT);
}

QString ExifReader::imageDescription() const 
{
	return stringTag(EXIF_TAG_IMAGE_DESCRIPTION);
}

QDateTime ExifReader::creationTime() 
{
	QDateTime dateTime;
	if (!m_data)
		return dateTime;

	const ExifTag TIME_TAGS[3] = {
		EXIF_TAG_DATE_TIME,
		EXIF_TAG_DATE_TIME_ORIGINAL,
		EXIF_TAG_DATE_TIME_DIGITIZED
	};
	for (unsigned i = 0; i < sizeof(TIME_TAGS) / sizeof(ExifTag); i++) {
		ExifEntry *entry = exif_data_get_entry(m_data, TIME_TAGS[i]);
		if (!entry)
			continue;
		dateTime = QDateTime::fromString(stringValue(entry), Qt::ISODate);
		if (dateTime.isValid())
			return dateTime;
	}

	return dateTime;
}

QString ExifReader::cameraModel() const 
{
	return stringTag(EXIF_TAG_MODEL);
}

QString ExifReader::cameraMake() const 
{
	return stringTag(EXIF_TAG_MAKE);
}

// http://www.sno.phy.queensu.ca/~phil/exiftool/TagNames/GPS.html
double ExifReader::latitude(bool *hasLatitude) const 
{
    if (hasLatitude)
        *hasLatitude = false;

	if (!m_data)
		return 0;

	ExifEntry *entry = exif_data_get_entry(m_data, (ExifTag) EXIF_TAG_GPS_LATITUDE_REF);
	const QString ref = stringValue(entry).trimmed().toUpper();
	if (ref != "N" && ref != "S")
		return 0;
	entry = exif_data_get_entry(m_data, (ExifTag) EXIF_TAG_GPS_LATITUDE);
	if (!entry)
		return 0;
	ExifRational degrees = exif_get_rational(entry->data + 0 * m_sizeOfRational, m_byteOrder);
	ExifRational minutes = exif_get_rational(entry->data + 1 * m_sizeOfRational, m_byteOrder);
	ExifRational seconds = exif_get_rational(entry->data + 2 * m_sizeOfRational, m_byteOrder);
	if (degrees.denominator == 0 || minutes.denominator == 0 || seconds.denominator == 0)
		return 0;
    
    if (hasLatitude)
        *hasLatitude = true;

	return (ref == "N" ? +1 : -1)
			* (degrees.numerator/degrees.denominator)
				+ (minutes.numerator/(minutes.denominator*60.0))
				+ (seconds.numerator/(seconds.denominator*3600.0));
}

double ExifReader::longitude(bool *hasLongitude) const 
{
    if (hasLongitude)
        *hasLongitude = false;

	if (!m_data)
		return 0;

	ExifEntry *entry = exif_data_get_entry(m_data, (ExifTag) EXIF_TAG_GPS_LONGITUDE_REF);
	const QString ref = stringValue(entry).trimmed().toUpper();
	if (ref != "E" && ref != "W")
		return 0;
	entry = exif_data_get_entry(m_data, (ExifTag) EXIF_TAG_GPS_LONGITUDE);
	if (!entry)
		return 0;
	ExifRational degrees = exif_get_rational(entry->data + 0 * m_sizeOfRational, m_byteOrder);
	ExifRational minutes = exif_get_rational(entry->data + 1 * m_sizeOfRational, m_byteOrder);
	ExifRational seconds = exif_get_rational(entry->data + 2 * m_sizeOfRational, m_byteOrder);
	if (degrees.denominator == 0 || minutes.denominator == 0 || seconds.denominator == 0)
		return 0;

    if (hasLongitude)
        *hasLongitude = true;

	return (ref == "E" ? +1 : -1)
			* ((degrees.numerator/degrees.denominator)
			+  (minutes.numerator/(minutes.denominator*60.0))
			+  (seconds.numerator/(seconds.denominator*3600.0)));
}

double ExifReader::altitude(bool *hasAltitude) const 
{
    if (hasAltitude)
        *hasAltitude = false;

	if (!m_data)
		return 0;

	ExifEntry *entry = exif_data_get_entry(m_data, (ExifTag) EXIF_TAG_GPS_ALTITUDE_REF);
	if (!entry)
		return 0;
	char ref;
	exif_entry_get_value(entry, &ref, 1);
	entry = exif_data_get_entry(m_data, (ExifTag) EXIF_TAG_GPS_ALTITUDE);
	if (!entry)
		return 0;
	ExifRational r = exif_get_rational(entry->data, m_byteOrder);
	if (r.denominator == 0)
		return 0;

    if (hasAltitude)
        *hasAltitude = true;

	return (ref == 0 ? +1 : -1)  * (r.numerator*1.0/r.denominator);
}

ExifReader::Orientation ExifReader::orientation() const 
{
	if (!m_data)
		return Invalid;

	ExifEntry *entry = exif_data_get_entry(m_data, (ExifTag) EXIF_TAG_ORIENTATION);
	if (!entry)
		return Invalid;
	ExifByteOrder order = exif_data_get_byte_order(m_data);
	return (Orientation) exif_get_short (entry->data, order);
}

QString ExifReader::stringValue(ExifEntry *entry) const 
{
	if (!entry)
		return QString();

	char value[1024] = { 0 };
	exif_entry_get_value(entry, value, sizeof(value) - 1);
	value[sizeof(value)-1] = 0;
	return QString::fromAscii(value); // ###: fix encoding
}

QString ExifReader::aperture() const
{
    return stringTag(EXIF_TAG_APERTURE_VALUE);
}

QString ExifReader::focalLength() const
{
    return stringTag(EXIF_TAG_FOCAL_LENGTH);
}

QString ExifReader::exposureTime() const
{
    return stringTag(EXIF_TAG_EXPOSURE_TIME);
}

QString ExifReader::exposureMode() const
{
    return stringTag(EXIF_TAG_EXPOSURE_MODE);
}

QString ExifReader::whiteBalance() const
{
    return stringTag(EXIF_TAG_WHITE_BALANCE);
}

QString ExifReader::lightSource() const
{
    return stringTag(EXIF_TAG_LIGHT_SOURCE);
}

QString ExifReader::isoSpeed() const
{
    return stringTag(EXIF_TAG_ISO_SPEED_RATINGS);
}

QString ExifReader::digitalZoomRatio() const
{
    return stringTag(EXIF_TAG_DIGITAL_ZOOM_RATIO);
}

QString ExifReader::flashUsage() const
{
    return stringTag(EXIF_TAG_FLASH);
}

QString ExifReader::colorSpace() const
{
    return stringTag(EXIF_TAG_COLOR_SPACE);
}



