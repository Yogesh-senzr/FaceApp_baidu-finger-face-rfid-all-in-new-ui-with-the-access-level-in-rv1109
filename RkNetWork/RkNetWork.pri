INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

QT += core
CONFIG += c++11

HEADERS += \
    $$PWD/NetworkControlThread.h \
    $$PWD/Network4GControlThread.h

SOURCES += \
    $$PWD/NetworkControlThread.cpp \
    $$PWD/Network4GControlThread.cpp

LIBS += -L$(CROOT)/buildroot/output/rockchip_rv1126_rv1109_facial_gate/target/usr/lib/ -lIPCProtocol

