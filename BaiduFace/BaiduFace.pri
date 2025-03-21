QT += core
#CONFIG += c++11 

##CPPFLAG += "-fPIC -Ofast -ldl -fstack-protector -fstack-check -Wl,-z,relro,-z,now -fvisibility=hidden"

INCLUDEPATH += $$PWD
INCLUDEPATH += $$PWD/inc
DEPENDPATH += $$PWD
INCLUDEPATH += $$PWD/inc
DEPENDPATH += $$PWD/inc

HEADERS += \
    $$PWD/BaiduFaceManager.h \
    $$PWD/CBaiduFaceEngine.h \    


SOURCES += \
    $$PWD/CBaiduFaceEngine.cpp \
    $$PWD/BaiduFaceManager.cpp \

LIBS += -L$$PWD/lib -lfaceid -lmot_sort

INCLUDEPATH += $$PWD/.
INCLUDEPATH += $$PWD/inc
DEPENDPATH += $$PWD/.

##LIBS += -L$(CROOT)/prebuilts/gcc/linux-x86/arm/gcc-arm-8.3-2019.03-x86_64-arm-linux-gnueabihf/arm-linux-gnueabihf/lib/ -lstdc++
###LIBS += -L$(CROOT)/prebuilts/gcc/linux-x86/arm/gcc-arm-8.3-2019.03-x86_64-arm-linux-gnueabihf/arm-linux-gnueabihf/lib -lstdc++
