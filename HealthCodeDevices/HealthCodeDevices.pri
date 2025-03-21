unix{
HEADERS += \
    $$PWD/DKHealthCode.h \
    $$PWD/LRHealthCode.h \    
    $$PWD/HealthCodeManage.h

SOURCES += \
    $$PWD/DKHealthCode.cpp \  
    $$PWD/LRHealthCode.cpp \     
    $$PWD/HealthCodeManage.cpp

}else{
HEADERS += \
    $$PWD/HealthCodeManage.h

SOURCES += \
    $$PWD/HealthCodeManage.cpp
}

unix{
    LIBS += -L$(CROOT)/buildroot/output/rockchip_rv1126_rv1109_facial_gate/target/usr/lib/ -lcurl
    LIBS += -L$(CROOT)/buildroot/output/rockchip_rv1126_rv1109_facial_gate/target/usr/lib/ -lssl
    LIBS += -L$(CROOT)/buildroot/output/rockchip_rv1126_rv1109_facial_gate/target/usr/lib/ -lcrypto
}
