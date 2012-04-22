DEFINES += NDEBUG WITH_ASF WITH_MP4 TAGLIB_NO_CONFIG
contains(QT_CONFIG, system-zlib) {
    LIBS += -lz
    HAVE_ZLIB=1
} else:contains(QT_CONFIG, zlib) {
    HAVE_ZLIB=1
} else {
    message(taglib could not detect zlib in Qt or system)
}
VERSION = 1.6.3
DEPENDPATH += $$PWD \
           $$PWD/ape \
           $$PWD/asf \
           $$PWD/flac \
           $$PWD/mp4 \
           $$PWD/mpc \
           $$PWD/mpeg \
           $$PWD/mpeg/id3v1 \
           $$PWD/mpeg/id3v2 \
           $$PWD/mpeg/id3v2/frames \
           $$PWD/ogg \
           $$PWD/ogg/flac \
           $$PWD/ogg/speex \
           $$PWD/ogg/vorbis \
           $$PWD/riff \
           $$PWD/riff/aiff \
           $$PWD/riff/wav \
           $$PWD/toolkit \
           $$PWD/trueaudio \
           $$PWD/wavpack

INCLUDEPATH += $$PWD \
           $$PWD/ape \
           $$PWD/asf \
           $$PWD/flac \
           $$PWD/mp4 \
           $$PWD/mpc \
           $$PWD/mpeg \
           $$PWD/mpeg/id3v1 \
           $$PWD/mpeg/id3v2 \
           $$PWD/mpeg/id3v2/frames \
           $$PWD/ogg \
           $$PWD/ogg/flac \
           $$PWD/ogg/speex \
           $$PWD/ogg/vorbis \
           $$PWD/riff \
           $$PWD/riff/aiff \
           $$PWD/riff/wav \
           $$PWD/toolkit \
           $$PWD/trueaudio \
           $$PWD/wavpack

# Input
HEADERS += $$PWD/audioproperties.h \
           $$PWD/fileref.h \
           $$PWD/tag.h \
           $$PWD/taglib_export.h \
           $$PWD/tagunion.h \
           $$PWD/ape/apefile.h \
           $$PWD/ape/apefooter.h \
           $$PWD/ape/apeitem.h \
           $$PWD/ape/apeproperties.h \
           $$PWD/ape/apetag.h \
           $$PWD/flac/flacfile.h \
           $$PWD/flac/flacpicture.h \
           $$PWD/flac/flacproperties.h \
           $$PWD/mpc/mpcfile.h \
           $$PWD/mpc/mpcproperties.h \
           $$PWD/mp4/mp4atom.h \
           $$PWD/mp4/mp4item.h \
           $$PWD/mp4/mp4file.h \
           $$PWD/mp4/mp4properties.h \
           $$PWD/mpeg/mpegfile.h \
           $$PWD/mpeg/mpegheader.h \
           $$PWD/mpeg/mpegproperties.h \
           $$PWD/mpeg/xingheader.h \
           $$PWD/ogg/oggfile.h \
           $$PWD/ogg/oggpage.h \
           $$PWD/ogg/oggpageheader.h \
           $$PWD/ogg/xiphcomment.h \
           $$PWD/ogg/speex/speexfile.h \
           $$PWD/ogg/speex/speexproperties.h \
           $$PWD/toolkit/taglib.h \
           $$PWD/toolkit/tbytevector.h \
           $$PWD/toolkit/tbytevectorlist.h \
           $$PWD/toolkit/tdebug.h \
           $$PWD/toolkit/tfile.h \
           $$PWD/toolkit/tlist.h \
           $$PWD/toolkit/tmap.h \
           $$PWD/toolkit/tstring.h \
           $$PWD/toolkit/tstringlist.h \
           $$PWD/toolkit/unicode.h \
           $$PWD/trueaudio/trueaudiofile.h \
           $$PWD/trueaudio/trueaudioproperties.h \
           $$PWD/wavpack/wavpackfile.h \
           $$PWD/wavpack/wavpackproperties.h \
           $$PWD/mpeg/id3v1/id3v1genres.h \
           $$PWD/mpeg/id3v1/id3v1tag.h \
           $$PWD/mpeg/id3v2/id3v2extendedheader.h \
           $$PWD/mpeg/id3v2/id3v2footer.h \
           $$PWD/mpeg/id3v2/id3v2frame.h \
           $$PWD/mpeg/id3v2/id3v2framefactory.h \
           $$PWD/mpeg/id3v2/id3v2header.h \
           $$PWD/mpeg/id3v2/id3v2synchdata.h \
           $$PWD/mpeg/id3v2/id3v2tag.h \
           $$PWD/ogg/flac/oggflacfile.h \
           $$PWD/ogg/vorbis/vorbisfile.h \
           $$PWD/ogg/vorbis/vorbisproperties.h \
           $$PWD/mpeg/id3v2/frames/attachedpictureframe.h \
           $$PWD/mpeg/id3v2/frames/commentsframe.h \
           $$PWD/mpeg/id3v2/frames/generalencapsulatedobjectframe.h \
           $$PWD/mpeg/id3v2/frames/popularimeterframe.h \
           $$PWD/mpeg/id3v2/frames/relativevolumeframe.h \
           $$PWD/mpeg/id3v2/frames/textidentificationframe.h \
           $$PWD/mpeg/id3v2/frames/uniquefileidentifierframe.h \
           $$PWD/mpeg/id3v2/frames/unknownframe.h \
           $$PWD/mpeg/id3v2/frames/unsynchronizedlyricsframe.h \
           $$PWD/mpeg/id3v2/frames/urllinkframe.h \
           $$PWD/toolkit/tlist.tcc \
           $$PWD/toolkit/tmap.tcc
SOURCES += $$PWD/ape/apefile.cpp \
           $$PWD/ape/apefooter.cpp \
           $$PWD/ape/apeitem.cpp \
           $$PWD/ape/apeproperties.cpp \
           $$PWD/ape/apetag.cpp \
           $$PWD/asf/asfattribute.cpp \
           $$PWD/asf/asffile.cpp \
           $$PWD/asf/asfproperties.cpp \
           $$PWD/asf/asftag.cpp \
           $$PWD/audioproperties.cpp \
           $$PWD/fileref.cpp \
           $$PWD/flac/flacfile.cpp \
           $$PWD/flac/flacpicture.cpp \
           $$PWD/flac/flacproperties.cpp \
           $$PWD/mp4/mp4atom.cpp \
           $$PWD/mp4/mp4coverart.cpp \
           $$PWD/mp4/mp4file.cpp \
           $$PWD/mp4/mp4item.cpp \
           $$PWD/mp4/mp4properties.cpp \
           $$PWD/mp4/mp4tag.cpp \
           $$PWD/mpc/mpcfile.cpp \
           $$PWD/mpc/mpcproperties.cpp \
           $$PWD/mpeg/id3v1/id3v1genres.cpp \
           $$PWD/mpeg/id3v1/id3v1tag.cpp \
           $$PWD/mpeg/id3v2/frames/attachedpictureframe.cpp \
           $$PWD/mpeg/id3v2/frames/commentsframe.cpp \
           $$PWD/mpeg/id3v2/frames/generalencapsulatedobjectframe.cpp \
           $$PWD/mpeg/id3v2/frames/popularimeterframe.cpp \
           $$PWD/mpeg/id3v2/frames/privateframe.cpp \
           $$PWD/mpeg/id3v2/frames/relativevolumeframe.cpp \
           $$PWD/mpeg/id3v2/frames/textidentificationframe.cpp \
           $$PWD/mpeg/id3v2/frames/uniquefileidentifierframe.cpp \
           $$PWD/mpeg/id3v2/frames/unknownframe.cpp \
           $$PWD/mpeg/id3v2/frames/unsynchronizedlyricsframe.cpp \
           $$PWD/mpeg/id3v2/frames/urllinkframe.cpp \
           $$PWD/mpeg/id3v2/id3v2extendedheader.cpp \
           $$PWD/mpeg/id3v2/id3v2footer.cpp \
           $$PWD/mpeg/id3v2/id3v2frame.cpp \
           $$PWD/mpeg/id3v2/id3v2framefactory.cpp \
           $$PWD/mpeg/id3v2/id3v2header.cpp \
           $$PWD/mpeg/id3v2/id3v2synchdata.cpp \
           $$PWD/mpeg/id3v2/id3v2tag.cpp \
           $$PWD/mpeg/mpegfile.cpp \
           $$PWD/mpeg/mpegheader.cpp \
           $$PWD/mpeg/mpegproperties.cpp \
           $$PWD/mpeg/xingheader.cpp \
           $$PWD/ogg/flac/oggflacfile.cpp \
           $$PWD/ogg/oggfile.cpp \
           $$PWD/ogg/oggpage.cpp \
           $$PWD/ogg/oggpageheader.cpp \
           $$PWD/ogg/speex/speexfile.cpp \
           $$PWD/ogg/speex/speexproperties.cpp \
           $$PWD/ogg/vorbis/vorbisfile.cpp \
           $$PWD/ogg/vorbis/vorbisproperties.cpp \
           $$PWD/ogg/xiphcomment.cpp \
           $$PWD/riff/aiff/aifffile.cpp \
           $$PWD/riff/aiff/aiffproperties.cpp \
           $$PWD/riff/rifffile.cpp \
           $$PWD/riff/wav/wavfile.cpp \
           $$PWD/riff/wav/wavproperties.cpp \
           $$PWD/tag.cpp \
           $$PWD/tagunion.cpp \
           $$PWD/toolkit/tbytevector.cpp \
           $$PWD/toolkit/tbytevectorlist.cpp \
           $$PWD/toolkit/tdebug.cpp \
           $$PWD/toolkit/tfile.cpp \
           $$PWD/toolkit/tstring.cpp \
           $$PWD/toolkit/tstringlist.cpp \
           $$PWD/toolkit/unicode.cpp \
           $$PWD/trueaudio/trueaudiofile.cpp \
           $$PWD/trueaudio/trueaudioproperties.cpp \
           $$PWD/wavpack/wavpackfile.cpp \
           $$PWD/wavpack/wavpackproperties.cpp

