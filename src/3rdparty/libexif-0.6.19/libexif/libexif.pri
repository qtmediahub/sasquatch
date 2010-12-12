#IF(WIN32)
#	configure_file(${CMAKE_CURRENT_SOURCE_DIR}/_config-exif-win.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/_config-exif.h)
#ELSE(WIN32)
#	configure_file(${CMAKE_CURRENT_SOURCE_DIR}/_config-exif.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/_config-exif.h)
#ENDIF(WIN32)

DEFINES += GETTEXT_PACKAGE=\\\"libexif-12\\\"
DEFINES += HAVE_CONFIG_H=0

SOURCES += $$PWD/exif-byte-order.c \
           $$PWD/exif-content.c \
           $$PWD/exif-data.c \
           $$PWD/exif-entry.c \
           $$PWD/exif-format.c \
           $$PWD/exif-ifd.c \
           $$PWD/exif-loader.c \
           $$PWD/exif-log.c \
           $$PWD/exif-mem.c \
           $$PWD/exif-mnote-data.c \
           $$PWD/exif-tag.c \
           $$PWD/exif-utils.c \
           $$PWD/canon/exif-mnote-data-canon.c \
           $$PWD/canon/mnote-canon-entry.c \
           $$PWD/canon/mnote-canon-tag.c \
           $$PWD/fuji/exif-mnote-data-fuji.c \
           $$PWD/fuji/mnote-fuji-entry.c \
           $$PWD/fuji/mnote-fuji-tag.c \
           $$PWD/olympus/exif-mnote-data-olympus.c \
           $$PWD/olympus/mnote-olympus-entry.c \
           $$PWD/olympus/mnote-olympus-tag.c \
           $$PWD/pentax/exif-mnote-data-pentax.c \
           $$PWD/pentax/mnote-pentax-entry.c \
           $$PWD/pentax/mnote-pentax-tag.c

