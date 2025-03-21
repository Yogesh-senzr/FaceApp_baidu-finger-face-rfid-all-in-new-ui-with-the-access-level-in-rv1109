unix{
HEADERS += \
    $$PWD/derkiot/tcp_client.h \
    $$PWD/derkiot/uart.h \
    $$PWD/derkiot/utf.h \
    $$PWD/derkiot/get_bmp_img.h \
    $$PWD/derkiot/id_data_decode.h \
    $$PWD/derkiot/ares.h \
    $$PWD/derkiot/ares_build.h \
    $$PWD/derkiot/ares_dns.h \
    $$PWD/derkiot/ares_rules.h \
    $$PWD/derkiot/ares_version.h \
    $$PWD/derkiot/dk_c_ares.h \
    $$PWD/derkiot/dk_utils.h \
    $$PWD/derkiot/DKReader.h \
    $$PWD/DKIdentityCard.h \
    $$PWD/IdentityCardManage.h

SOURCES += \
    $$PWD/derkiot/tcp_client.c \
    $$PWD/derkiot/uart.c \
    $$PWD/derkiot/utf.c \
    $$PWD/derkiot/get_bmp_img.c \
    $$PWD/derkiot/id_data_decode.c \
    $$PWD/derkiot/dk_c_ares.c \
    $$PWD/derkiot/dk_utils.c \
    $$PWD/derkiot/DKReader.c \
    $$PWD/DKIdentityCard.cpp \
    $$PWD/IdentityCardManage.cpp

}else{
HEADERS += \
    $$PWD/IdentityCardManage.h

SOURCES += \
    $$PWD/IdentityCardManage.cpp
}

unix{
    LIBS += -L$$PWD/Libs/ -lcares
    LIBS += -L$$PWD/Libs/ -lSynReaderArm
    LIBS += -L$$PWD/Libs/ -lwlt

    INCLUDEPATH += $$PWD/Libs/.
    DEPENDPATH += $$PWD/Libs/.
}
