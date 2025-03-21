INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

QT += core
CONFIG += c++11

HEADERS += $$PWD/Camera/aiq_control.h \
    $$PWD/Camera/cameramanager.h \
    $$PWD/Camera/camir_control.h \
    $$PWD/Camera/camrgb_control.h \
    $$PWD/Drm/amcomdef.h \
    $$PWD/Drm/display.h \
    $$PWD/Drm/draw_rect.h \
    $$PWD/Drm/rkdrm_display.h

SOURCES += $$PWD/Camera/aiq_control.cpp \
    $$PWD/Camera/cameramanager.cpp \
    $$PWD/Camera/camir_control.c \
    $$PWD/Camera/camrgb_control.c \
    $$PWD/Drm/display.c \
    $$PWD/Drm/draw_rect.c \
    $$PWD/Drm/rkdrm_display.c


SYSROOT = $(CROOT)/buildroot/output/rockchip_rv1126_rv1109_facial_gate/host/arm-buildroot-linux-gnueabihf/sysroot

INCPATH +=  $$SYSROOT/usr/include/rkaiq/
INCPATH +=  $$SYSROOT/usr/include/rkaiq/common/
INCPATH +=  $$SYSROOT/usr/include/rkaiq/xcore/
INCPATH +=  $$SYSROOT/usr/include/rkaiq/algos
INCPATH +=  $$SYSROOT/usr/include/rkaiq/iq_parser
INCPATH +=  $$SYSROOT/usr/include/rkfacial/
INCPATH +=  $$SYSROOT/usr/include/drm/

LIBS += -L$(CROOT)/buildroot/output/rockchip_rv1126_rv1109_facial_gate/target/usr/lib/ -lrga
LIBS += -L$(CROOT)/buildroot/output/rockchip_rv1126_rv1109_facial_gate/target/usr/lib/ -ldrm
LIBS += -L$(CROOT)/buildroot/output/rockchip_rv1126_rv1109_facial_gate/target/usr/lib/ -lrkfacial
LIBS += -L$(CROOT)/buildroot/output/rockchip_rv1126_rv1109_facial_gate/target/usr/lib/ -lrkaiq
LIBS += -L$(CROOT)/buildroot/output/rockchip_rv1126_rv1109_facial_gate/target/usr/lib/ -lrkisp_api
#LIBS += -L$(CROOT)/buildroot/output/rockchip_rv1126_rv1109_facial_gate/target/usr/lib/ -lcurl
#LIBS += -L$(CROOT)/buildroot/output/rockchip_rv1126_rv1109_facial_gate/target/usr/lib/ -lusb-1.0
#LIBS += -L$(CROOT)/buildroot/output/rockchip_rv1126_rv1109_facial_gate/target/usr/lib/ -lssl
#LIBS += -L$(CROOT)/buildroot/output/rockchip_rv1126_rv1109_facial_gate/target/usr/lib/ -lcrypto
LIBS += -L$(CROOT)/buildroot/output/rockchip_rv1126_rv1109_facial_gate/target/usr/lib/ -lturbojpeg
LIBS += -L$(CROOT)/buildroot/output/rockchip_rv1126_rv1109_facial_gate/target/usr/lib/ -lprotobuf
#LIBS += -L$(CROOT)/buildroot/output/rockchip_rv1126_rv1109_facial_gate/target/usr/lib/ -lpthread

