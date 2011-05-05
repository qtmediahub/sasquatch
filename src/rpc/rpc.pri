include(../3rdparty/jsonparser/json.pri)

INCLUDEPATH += 3rdparty

HEADERS += $$PWD/rpcconnection.h \
           $$PWD/mediaplayerrpc.h

SOURCES += $$PWD/rpcconnection.cpp \
           $$PWD/mediaplayerrpc.cpp

