unix{
HEADERS += \
    $$PWD/ZKIdentityCard.h \
    $$PWD/IdentityCard_ZK_Manage.h

SOURCES += \
    $$PWD/ZKIdentityCard.cpp \
    $$PWD/IdentityCard_ZK_Manage.cpp
}else{
HEADERS += \
    $$PWD/IdentityCard_ZK_Manage.h

SOURCES += \
    $$PWD/IdentityCard_ZK_Manage.cpp
}

unix{
    LIBS += -L$$PWD/Libs/ -lcares
    LIBS += -L$$PWD/Libs/ -lSynReaderArm
    ##LIBS += -L$$PWD/Libs/ -lwlt_ZK
    LIBS += -L$$PWD/Libs/ -lwlt
	LIBS += -L$$PWD/Libs/ -lzkident

    INCLUDEPATH += $$PWD/Libs/.
    DEPENDPATH += $$PWD/Libs/.
}
