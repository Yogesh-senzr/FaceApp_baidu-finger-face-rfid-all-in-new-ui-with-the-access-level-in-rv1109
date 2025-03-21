unix{
HEADERS += \
    $$PWD/EidSdkStruct.hpp \
    $$PWD/EsEidSdk.hpp \
	$$PWD/ISdkLog.hpp \
	$$PWD/IdentityCard_dd.h \
	$$PWD/IdentityCard_DD_Manage.h

SOURCES += \
    $$PWD/IdentityCard_dd.cpp \
    $$PWD/IdentityCard_DD_Manage.cpp
}

unix{
    LIBS += -L$$PWD/Libs -les_eid_sdk
    LIBS += -L$$PWD/Libs -les_mongoose
    LIBS += -L$$PWD/Libs -lhidapi-hidraw
    #LIBS += -L$$PWD/Libs -les_mongoose
    #LIBS += -L$$PWD/Libs -les_mongoose
    #LIBS += -L$$PWD/Libs -les_mongoose


    INCLUDEPATH += $$PWD/Libs/.
    DEPENDPATH += $$PWD/Libs/.
}
