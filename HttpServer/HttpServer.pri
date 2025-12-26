QT += core
CONFIG += c++11

INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

HEADERS += \
    $$PWD/local_service.h \
    $$PWD/protocol.h \
    $$PWD/ConnHttpServerThread.h \
    $$PWD/PostPersonRecordThread.h \
    $$PWD/UdpBroadcastThread.h \


SOURCES += \
    $$PWD/local_service.cpp \
    $$PWD/protocol.cpp \
    $$PWD/ConnHttpServerThread.cpp \
    $$PWD/PostPersonRecordThread.cpp \
    $$PWD/UdpBroadcastThread.cpp \


INCLUDEPATH += $$PWD/.
DEPENDPATH += $$PWD/.

LIBS += -L$$PWD/libs/ -lPAPI
