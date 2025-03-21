#include "IdentityManagementPrivate.h"
#include "FaceDataResolverObj.h"

IdentityManagementPrivate::IdentityManagementPrivate(IdentityManagement *dd)
    : q_ptr(dd)
    , mVivoDetection(true)//活体检测
    , mAlarmTemp(37.3)//报警温度
    , mMaskValue(0.25)//口罩阈值
    , mThanthreshold(0.8)//比对阈值
    , midcardThanthreshold(0.7)//身份证比对阈值
    , mIdentifyInterval(5)//识别间隔
    , mFqThreshold(0.5)
    , mCurTemperatureValue(0.0)
    , mIdentityCard({})
    , mHealthCode({})
    , micCard({})
    , mIdentifyFaceRecord({})
    ,mFirstTime(1)
{    
    FaceDataResolverObj * pNewObject = new FaceDataResolverObj;
    this->m_ThreadObjs.push_back(pNewObject);
    pNewObject->EchoFaceRecognition(CC_CALLBACK_12(IdentityManagement::EchoFaceRecognition, dd));


}

IdentityManagementPrivate::~IdentityManagementPrivate()
{

}


QString IdentityManagementPrivate::Door_OpenModeToSTR(const _DOOR_OPEN_MODE mode)
{
    switch(mode)
    {
    case _DOOR_OPEN_MODE::ICCARD:return QString("1");//刷卡
    case _DOOR_OPEN_MODE::SWIPING_FACE:return QString("2");//刷脸
    case _DOOR_OPEN_MODE::THERMOMETRY:return QString("3");//测温
    case _DOOR_OPEN_MODE::MASK:return QString("4");//口罩
    case _DOOR_OPEN_MODE::QRCODE:return QString("5");//二唯码
    case _DOOR_OPEN_MODE::IDCARD:return QString("6");//身份证
    case _DOOR_OPEN_MODE::PERSON_IDCARD:return QString("7");//人证比对
    case _DOOR_OPEN_MODE::QRCODE_LOCAL:return QString("8");//本地码,粤康码/深I您
    default:return QString("255");
    }
}
