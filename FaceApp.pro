QT       += core gui sql network
QT       += gui-private widgets

CONFIG += c++11
CONFIG += resources_big

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

target.path = /isc/bin
INSTALLS += target

TARGET = face_app
TEMPLATE = app

#指定生成临时文件路径
MOC_DIR         = temp/moc
RCC_DIR         = temp/rcc
UI_DIR          = temp/ui
OBJECTS_DIR     = temp/obj
DESTDIR         = ../FaceAppDebug

#屏蔽警告
QMAKE_CXXFLAGS +=  -Wno-unused-parameter
QMAKE_CXXFLAGS +=  -Wno-unused-function
QMAKE_CXXFLAGS +=  -Wno-unused-but-set-variable
QMAKE_CXXFLAGS +=  -Wno-reorder
QMAKE_CXXFLAGS +=  -Wno-format
QMAKE_CXXFLAGS +=  -Wno-delete-non-virtual-dtor
QMAKE_CXXFLAGS +=  -Wno-sign-compare
## gdb
#QMAKE_CXXFLAGS +=  -g  

##QMAKE_CXXFLAGS +=  -fPIE
##QMAKE_CXXFLAGS +=  -fPIC -Ofast -ldl -fstack-protector -fstack-check -Wl,-z,relro,-z,now -fvisibility=hidden  -std=c++11 -O3 -DNDEBUG 

#QMAKE_CXXFLAGS_RELEASE = $$QMAKE_CFLAGS_RELEASE_WITH_DEBUGINFO  
#QMAKE_LFLAGS_RELEASE = $$QMAKE_LFLAGS_RELEASE_WITH_DEBUGINFO  

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

## Screen Catpure function 
#DEFINES += SCREENCAPTURE

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

unix{
    include(RKCamera/RkCamera.pri)#RK相机
    include(RkNetWork/RkNetWork.pri)#配置网络及查找wifi
    include(BaseFace/BaseFace.pri) #抽象算法引擎      
    include(BaiduFace/BaiduFace.pri)#百度算法引擎      
    include(HttpServer/HttpServer.pri)
    include(PCIcore/PCIcore.pri)#外围设备， 温度传感器、身份证
    include(USB/USB.pri)#挂载USB
}
    include(QtXlsxWriter/src/xlsx/qtxlsx.pri)#读取Excel
    include(HealthCodeDevices/HealthCodeDevices.pri)#健康码
    include(IDCardDevices/IDCardDevices.pri)#身份证
    include(IDCard_ZK/IDCard_ZK.pri)#USB 身份证ZKSDK    
    include(IDCard_diandu/IDCard_dd.pri)#USB 身份证 点度 SDK    
    include(Zhangjiakou/Zhangjiakou.pri)#张家口后台
    include(HaoyuIdentificationCheck/HaoyuIdentificationCheck.pri)#浩宇平台
     

#张家口调试在win平台下库
win32{
    INCLUDEPATH += $$quote(C:/Program Files (x86)/OpenSSL-Win32/include)
    LIBS += -L"C:/Program Files (x86)/OpenSSL-Win32/lib" -llibcrypto
    LIBS += -L"C:/Program Files (x86)/OpenSSL-Win32/lib" -llibssl
}
INCLUDEPATH += QVirtualKeyboard/pinyin/include
SOURCES += \
        main.cpp \
        FaceMainFrm.cpp \
        DeviceInfo.cpp \
    Application/FaceApp.cpp \
    Helper/myhelper.cpp \
    FaceHomeFrms/FaceHomeFrm.cpp \
    SettingFuncFrms/HomeMenuFrm.cpp \
    SettingFuncFrms/SettingMenuFrm.cpp \
    SettingFuncFrms/PasswordDialog.cpp \
    SettingFuncFrms/SettingMenuTitleFrm.cpp \
    SettingFuncFrms/SettingEditTextFrm.cpp \    
    SettingFuncFrms/IdentifySetupFrms/IdentifySetupFrm.cpp \
    SettingFuncFrms/NetworkSetupFrms/NetworkSetupFrm.cpp \
    SettingFuncFrms/SrvSetupFrms/SrvSetupFrm.cpp \
    SettingFuncFrms/SysSetupFrms/SysSetupFrm.cpp \
    SettingFuncFrms/ManagingPeopleFrms/ManagingPeopleFrm.cpp \
    SettingFuncFrms/ManagingPeopleFrms/AddUserFrm.cpp \
    SettingFuncFrms/ManagingPeopleFrms/ImportUserFrm.cpp \
    SettingFuncFrms/ManagingPeopleFrms/UserViewFrm.cpp \
    SettingFuncFrms/RecordsManagementFrms/ViewAccessRecordsFrm.cpp \
    SettingFuncFrms/RecordsManagementFrms/RecordsManagementFrm.cpp \
    SettingFuncFrms/RecordsManagementFrms/ConfigAccessRecordsFrm.cpp \
    OperationTipsFrm.cpp \
    SettingFuncFrms/ManagingPeopleFrms/CameraPicFrm.cpp \
    SettingFuncFrms/SettingBaseFrm.cpp \
    SettingFuncFrms/NetworkSetupFrms/WifiViewFrm.cpp \
    SettingFuncFrms/NetworkSetupFrms/Network4GViewFrm.cpp \
    SettingFuncFrms/NetworkSetupFrms/EthernetViewFrm.cpp \
    SettingFuncFrms/NetworkSetupFrms/InputWifiPasswordFrm.cpp \
    SettingFuncFrms/SetupItemDelegate/CItemWifiWidget.cpp \
    SettingFuncFrms/SetupItemDelegate/CItemWidget.cpp \
    SettingFuncFrms/NetworkSetupFrms/ConfirmRestartFrm.cpp \
    SettingFuncFrms/SetupItemDelegate/CItemWifiBoxWidget.cpp \
    SettingFuncFrms/SetupItemDelegate/CItemBoxWidget.cpp \
    SettingFuncFrms/SysSetupFrms/LanguageFrm.cpp \
    SettingFuncFrms/SysSetupFrms/FillLightFrm.cpp \
    SettingFuncFrms/SysSetupFrms/LuminanceFrm.cpp \
    SettingFuncFrms/SysSetupFrms/VolumeFrm.cpp \
    SettingFuncFrms/SysSetupFrms/DoorControFrms/DoorControFrm.cpp \
    SettingFuncFrms/SysSetupFrms/AboutMachineFrm.cpp \
    SettingFuncFrms/SysSetupFrms/MainsetFrm.cpp \
    SettingFuncFrms/SysSetupFrms/StorageCapacityFrm.cpp \
    SettingFuncFrms/SysSetupFrms/TimesetFrm.cpp \
    SettingFuncFrms/SysSetupFrms/LogPasswordChangeFrm.cpp \
    SettingFuncFrms/SysSetupFrms/DoorControFrms/PassageTimeFrm.cpp \
    SettingFuncFrms/SysSetupFrms/DoorControFrms/AccessTypeFrm.cpp \
    SettingFuncFrms/SysSetupFrms/DoorControFrms/WigginsOutputFrm.cpp \
    SettingFuncFrms/SysSetupFrms/DoorControFrms/DoorLockFrm.cpp \
    SettingFuncFrms/SetupItemDelegate/CInputBaseDialog.cpp \
    Delegate/Numberdelegate.cpp \
    Delegate/RateModeDelegate.cpp \
    Delegate/Timedelegate.cpp \
    Delegate/wholedelegate.cpp \
    Delegate/CheckBoxDelegate.cpp \
    Delegate/Toast.cpp \
    SettingFuncFrms/SysSetupFrms/SystemMaintenanceFrm.cpp \
    SettingFuncFrms/SysSetupFrms/DisplayEffectFrm.cpp \
    SettingFuncFrms/SysSetupFrms/QRCodeFrm.cpp \
    SettingFuncFrms/SysSetupFrms/QRCodeGenerator.cpp \
    SettingFuncFrms/SysSetupFrms/SyncFunctionality.cpp \
    FaceHomeFrms/FaceHomeTitleFrm.cpp \
    FaceHomeFrms/FaceHomeBottomFrm.cpp \
    ManageEngines/IdentityManagement.cpp \
    ManageEngines/FaceDataResolverObj.cpp \
    DB/FaceDB.cpp \
    vmKeyboardInput/frminput.cpp \
    vmKeyboardInput/vitrualkeyboard.cpp \
    pagenavigator.cpp \
    Delegate/TableWidgetDelegate.cpp \
    SettingFuncFrms/InputLoginPasswordFrm.cpp \
    SettingFuncFrms/RecordsManagementFrms/FacePngFrm.cpp \
    Config/ReadConfig.cpp \
    SettingFuncFrms/SysSetupFrms/DoorControFrms/PassageModeFrm.cpp \
    DB/RegisteredFacesDB.cpp \
    ManageEngines/PersonRecordToDB.cpp \
    SystemMaintenanceManage.cpp \
    json-cpp/json_reader.cpp \
    json-cpp/json_value.cpp \
    json-cpp/json_writer.cpp \
    SettingFuncFrms/IdentifySetupFrms/IdentifyDistanceFrm.cpp \
    NtpDate/NtpDateSync.cpp \
    SettingFuncFrms/SysSetupFrms/DevicePoweroffFrm.cpp \
    Threads/powerManagerThread.cpp \
    Threads/ParsePersonXlsx.cpp \
    Threads/RecordsExport.cpp \
    Threads/ShrinkFaceImageThread.cpp \	
    FaceHomeFrms/HealthCodeFrm.cpp \
    Threads/WatchDogManageThread.cpp \
    MessageHandler/Log.cpp \
    SettingFuncFrms/IdentifySetupFrms/TemperatureModeFrm.cpp \
    SettingFuncFrms/SysSetupFrms/ChangeDateTimeFrm.cpp \
    SettingFuncFrms/RecordsManagementFrms/ViewAccessRecordsModel.cpp \
    SettingFuncFrms/ManagingPeopleFrms/UserViewModel.cpp \
    ManageEngines/HaoyuIdentityManagement.cpp \
    ManageEngines/IdentityManagementPrivate.cpp \
    ManageEngines/ZhangjiakoudentityManagement.cpp \
    FaceHomeFrms/HomeBottomBaseFrm.cpp \
    Delegate/ButtonDelegate.cpp

HEADERS += \
        FaceMainFrm.h \
        DeviceInfo.h \
        except.h \
    base64.hpp \
    Application/FaceApp.h \
    Helper/myhelper.h \
    FaceHomeFrms/FaceHomeFrm.h \
    SettingFuncFrms/HomeMenuFrm.h \
    SettingFuncFrms/PasswordDialog.h \
    SettingFuncFrms/SettingMenuFrm.h \
    SettingFuncFrms/SettingMenuTitleFrm.h \
    SettingFuncFrms/SettingEditTextFrm.h \    
    SettingFuncFrms/IdentifySetupFrms/IdentifySetupFrm.h \
    SettingFuncFrms/NetworkSetupFrms/NetworkSetupFrm.h \
    SettingFuncFrms/SrvSetupFrms/SrvSetupFrm.h \
    SettingFuncFrms/SysSetupFrms/SysSetupFrm.h \
    SettingFuncFrms/ManagingPeopleFrms/ManagingPeopleFrm.h \
    SettingFuncFrms/ManagingPeopleFrms/AddUserFrm.h \
    SettingFuncFrms/ManagingPeopleFrms/ImportUserFrm.h \
    SettingFuncFrms/ManagingPeopleFrms/UserViewFrm.h \
    SettingFuncFrms/RecordsManagementFrms/RecordsManagementFrm.h \
    SettingFuncFrms/RecordsManagementFrms/ViewAccessRecordsFrm.h \
    SettingFuncFrms/RecordsManagementFrms/ConfigAccessRecordsFrm.h \
    OperationTipsFrm.h \
    SettingFuncFrms/ManagingPeopleFrms/CameraPicFrm.h \
    SettingFuncFrms/SettingBaseFrm.h \
    SettingFuncFrms/NetworkSetupFrms/WifiViewFrm.h \
    SettingFuncFrms/NetworkSetupFrms/Network4GViewFrm.h \
    SettingFuncFrms/NetworkSetupFrms/EthernetViewFrm.h \
    SettingFuncFrms/NetworkSetupFrms/InputWifiPasswordFrm.h \
    SettingFuncFrms/SetupItemDelegate/CItemWifiWidget.h \
    SettingFuncFrms/SetupItemDelegate/CItemWidget.h \
    SettingFuncFrms/NetworkSetupFrms/ConfirmRestartFrm.h \
    SettingFuncFrms/SetupItemDelegate/CItemWifiBoxWidget.h \
    SettingFuncFrms/SetupItemDelegate/CItemBoxWidget.h \
    SettingFuncFrms/SysSetupFrms/LanguageFrm.h \
    SettingFuncFrms/SysSetupFrms/FillLightFrm.h \
    SettingFuncFrms/SysSetupFrms/LuminanceFrm.h \
    SettingFuncFrms/SysSetupFrms/VolumeFrm.h \
    SettingFuncFrms/SysSetupFrms/DoorControFrms/DoorControFrm.h \
    SettingFuncFrms/SysSetupFrms/AboutMachineFrm.h \
    SettingFuncFrms/SysSetupFrms/MainsetFrm.h \
    SettingFuncFrms/SysSetupFrms/StorageCapacityFrm.h \
    SettingFuncFrms/SysSetupFrms/TimesetFrm.h \
    SettingFuncFrms/SysSetupFrms/LogPasswordChangeFrm.h \
    SettingFuncFrms/SysSetupFrms/DoorControFrms/PassageTimeFrm.h \
    SettingFuncFrms/SysSetupFrms/DoorControFrms/AccessTypeFrm.h \
    SettingFuncFrms/SysSetupFrms/DoorControFrms/WigginsOutputFrm.h \
    SettingFuncFrms/SysSetupFrms/DoorControFrms/DoorLockFrm.h \    
    SettingFuncFrms/SetupItemDelegate/CInputBaseDialog.h \
    Delegate/Numberdelegate.h \
    Delegate/RateModeDelegate.h \
    Delegate/Timedelegate.h \
    Delegate/wholedelegate.h \
    Delegate/CheckBoxDelegate.h \
    Delegate/Toast.h \
    SettingFuncFrms/SysSetupFrms/SystemMaintenanceFrm.h \
    SettingFuncFrms/SysSetupFrms/DisplayEffectFrm.h \
    SettingFuncFrms/SysSetupFrms/QRCodeFrm.h \
    SettingFuncFrms/SysSetupFrms/QRCodeGenerator.h \
    SettingFuncFrms/SysSetupFrms/SyncFunctionality.h \
    FaceHomeFrms/FaceHomeTitleFrm.h \
    FaceHomeFrms/FaceHomeBottomFrm.h \
    SharedInclude/CallBindDef.h \
    SharedInclude/GlobalDef.h \
    ManageEngines/IdentityManagement.h \
    ManageEngines/FaceDataResolverObj.h \
    DB/FaceDB.h \
    DB/dbtable.h \
    vmKeyboardInput/frminput.h \
    vmKeyboardInput/vitrualkeyboard.h \
    pagenavigator.h \
    Delegate/TableWidgetDelegate.h \
    SettingFuncFrms/InputLoginPasswordFrm.h \
    SettingFuncFrms/RecordsManagementFrms/FacePngFrm.h \
    Config/ReadConfig.h \
    SettingFuncFrms/SysSetupFrms/DoorControFrms/PassageModeFrm.h \
    DB/RegisteredFacesDB.h \
    ManageEngines/PersonRecordToDB.h \
    SystemMaintenanceManage.h \
    json-cpp/allocator.h \
    json-cpp/assertions.h \
    json-cpp/autolink.h \
    json-cpp/config.h \
    json-cpp/features.h \
    json-cpp/forwards.h \
    json-cpp/json.h \
    json-cpp/json_tool.h \
    json-cpp/reader.h \
    json-cpp/value.h \
    json-cpp/version.h \
    json-cpp/writer.h \
    SettingFuncFrms/IdentifySetupFrms/IdentifyDistanceFrm.h \
    NtpDate/NtpDateSync.h \
    sliderclick.h \
    SettingFuncFrms/SysSetupFrms/DevicePoweroffFrm.h \
    Threads/powerManagerThread.h \
    Threads/ParsePersonXlsx.h \
    Threads/RecordsExport.h \
    Threads/ShrinkFaceImageThread.h \
    FaceHomeFrms/HealthCodeFrm.h \
    Threads/WatchDogManageThread.h \
    MessageHandler/Log.h \
    SettingFuncFrms/IdentifySetupFrms/TemperatureModeFrm.h \
    SettingFuncFrms/SysSetupFrms/ChangeDateTimeFrm.h \
    SettingFuncFrms/RecordsManagementFrms/ViewAccessRecordsModel.h \
    SettingFuncFrms/ManagingPeopleFrms/UserViewModel.h \
    ManageEngines/HaoyuIdentityManagement.h \
    ManageEngines/IdentityManagementPrivate.h \
    ManageEngines/ZhangjiakoudentityManagement.h \
    FaceHomeFrms/HomeBottomBaseFrm.h \
    Delegate/ButtonDelegate.h

RESOURCES += \
    images.qrc \
    css.qrc
#多语言
TRANSLATIONS += Ts/innohi_zh.ts \
    Ts/innohi_en.ts


## UTF-8; GB2312
CODECFORTR=UTF-8

FORMS += \
    vmKeyboardInput/frminput.ui \
    vmKeyboardInput/vitrualkeyboard.ui \
    pagenavigator.ui \

#LIBS += -L$(CROOT)/prebuilts/gcc/linux-x86/arm/gcc-arm-8.3-2019.03-x86_64-arm-linux-gnueabihf/arm-linux-gnueabihf/lib/  -lstdc++

INCLUDEPATH += /home/mathan/maaaatttttu/rv1126_rv1109_linux_sdk_release_20240831/buildroot/output/rockchip_rv1126_rv1109_facial_gate/host/arm-buildroot-linux-gnueabihf/sysroot/usr/include
LIBS += -L/home/mathan/maaaatttttu/rv1126_rv1109_linux_sdk_release_20240831/buildroot/output/rockchip_rv1126_rv1109_facial_gate/host/arm-buildroot-linux-gnueabihf/sysroot/usr/lib -lqrencode

