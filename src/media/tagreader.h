#ifndef TAGREADER_H
#define TAGREADER_H

#include <taglib/fileref.h>
#include <taglib/tag.h>
#include <taglib/mpegfile.h>
#include <taglib/id3v2tag.h>
#include <taglib/attachedpictureframe.h>

#include <QString>
#include <QImage>

class TagReader
{
public:
    TagReader(const QString &file);
    ~TagReader();

    bool isValid() const;
    bool hasAudioProperties() const;

    QString title() const;
    QString album() const;
    QString artist() const;
    int track() const;
    int year() const;
    QString genre() const;
    QString comment() const;

    int length() const;
    int bitrate() const;
    int sampleRate() const;
    int channels() const;

    QImage thumbnailImage() const;
    QByteArray thumbnail() const;

private:
    TagLib::FileRef m_fileRef;
    TagLib::Tag *m_tag;
    TagLib::AudioProperties *m_audioProperties;
};

#endif // TAGREADER_H

