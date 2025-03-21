QT += core
CONFIG += c++11

INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

HEADERS += \
    $$PWD/BaseFaceManager.h \

SOURCES += \
    $$PWD/BaseFaceManager.cpp \

INCLUDEPATH += $$PWD/.
DEPENDPATH += $$PWD/.

##LIBS += -L$(CROOT)/prebuilts/gcc/linux-x86/arm/gcc-linaro-6.3.1-2017.05-x86_64_arm-linux-gnueabihf/arm-linux-gnueabihf/lib/ -lstdc++
