#include "HaoyuIdentityManagement.h"
#include "IdentityManagementPrivate.h"
#ifdef Q_OS_LINUX
#include "PCIcore/Audio.h"
#include "PCIcore/Utils_Door.h"
#include "BaseFace/BaseFaceManager.h"
#endif

#include "HaoyuIdentificationCheck/HaoyuIdentificationCheck.h"
#include "Threads/powerManagerThread.h"
#include "FaceDataResolverObj.h"
#include "DB/RegisteredFacesDB.h"
#include "PersonRecordToDB.h"
#include "Application/FaceApp.h"
#include "Helper/myhelper.h"

#include <QDebug>
#include <QDateTime>

class HaoyuIdentityManagementPrivate : public IdentityManagementPrivate
{
    Q_DECLARE_PUBLIC(HaoyuIdentityManagement)
public:
    HaoyuIdentityManagementPrivate(HaoyuIdentityManagement *dd);
private://检测开门方式
    bool DistributeTheTasks();
private:
    HaoyuIdentificationCheck *m_pHaoyuIdentificationCheck;
};

HaoyuIdentityManagementPrivate::HaoyuIdentityManagementPrivate(HaoyuIdentityManagement *dd)
    : IdentityManagementPrivate(dd)
    , m_pHaoyuIdentificationCheck(new HaoyuIdentificationCheck)
{
}

HaoyuIdentityManagement::HaoyuIdentityManagement(QObject *parent)
    : IdentityManagement(new HaoyuIdentityManagementPrivate(this), parent)
{
    QObject::connect(d_func()->m_pHaoyuIdentificationCheck, &HaoyuIdentificationCheck::sigUserAuthResult, this, &HaoyuIdentityManagement::slotUserAuthResult, Qt::QueuedConnection);
    QObject::connect(this, &HaoyuIdentityManagement::sigPostFaceInfo, d_func()->m_pHaoyuIdentificationCheck, &HaoyuIdentificationCheck::slotPostFaceInfo, Qt::QueuedConnection);
}

HaoyuIdentityManagement::~HaoyuIdentityManagement()
{

}

void HaoyuIdentityManagement::slotUserAuthResult(const bool state, const QString msg)
{
    Q_D(HaoyuIdentityManagement);

    if(d->mIdentifyFaceRecord.face.enFaceType == CORE_FACE_RECT_TYPE_UNKNOW)return;
    else if(state == 0)
    {
#ifdef Q_OS_LINUX
        qXLApp->GetPowerManagerThread()->setIdentifyState(true);
        qXLApp->GetAlgoFaceManager()->setIdentifyState(true);

        YNH_LJX::Audio::Audio_PlayRecognizedPcm("zh");
        YNH_LJX::Utils_Door::GetInstance()->OpenDoor(d->mIdentifyFaceRecord.face_iccardnum);
#endif
        d->mIdentifyFaceRecord.FaceType = NOT_STRANGER;

        emit sigTipsMessage(BOTTOM_MESSAGE, 3, QObject::tr("Welcome"));//欢迎
    }else
    {//陌生人
#ifdef Q_OS_LINUX
        YNH_LJX::Audio::Audio_PlayPeopleStrangerPcm("zh");
#endif
        emit sigTipsMessage(BOTTOM_MESSAGE, 3, QObject::tr("<font color=\"#FFFF33\">stranger</font>"));//陌生人
    }

    emit sigTipsMessage(TOP_MESSAGE, 1, msg);
    d->mIdentifyFaceRecord.face.enFaceType = CORE_FACE_RECT_TYPE_DELETE;
    d->mIdentifyFaceRecord.time_Start = d->mIdentifyFaceRecord.time_End = (double)clock();
    PersonRecordToDB::GetInstance()->appRecordData(d->mIdentifyFaceRecord);
}

bool HaoyuIdentityManagementPrivate::DistributeTheTasks()
{
    switch((int)this->mIdentifyFaceRecord.face.enFaceType)
    {
    case CORE_FACE_RECT_TYPE_UNKNOW://没有人脸图
        return false;
    case CORE_FACE_RECT_TYPE_MOVING:
    {
        if(this->mMustOpenMode.contains("2"))//刷脸检测人员库
        {//查找人脸
            this->mIdentifyFaceRecord.face.enFaceType = CORE_FACE_RECT_TYPE_SEARCH;
            m_ThreadObjs.at(0)->SafeResolverData(this->mIdentifyFaceRecord.face);
        }
        else this->mIdentifyFaceRecord.face.enFaceType = CORE_FACE_RECT_TYPE_MATCH;
    }break;
    case CORE_FACE_RECT_TYPE_SEARCH://正查找人脸
        break;
    case CORE_FACE_RECT_TYPE_MATCH:
    {//人脸查找完成，检测开门模式是否达标
        qXLApp->GetPowerManagerThread()->setIdentifyState((this->mIdentifyFaceRecord.FaceType == NOT_STRANGER) ? true : false);
#ifdef Q_OS_LINUX
        /*改变识别状态*/
        qXLApp->GetAlgoFaceManager()->setIdentifyState((this->mIdentifyFaceRecord.FaceType == NOT_STRANGER) ? true : false);
#endif
        int passtimer = ((double)clock()- this->mIdentifyFaceRecord.time_Start)/1000/1000;
        if(this->mIdentifyInterval &&  (passtimer > this->mIdentifyInterval) && (this->mIdentifyFaceRecord.FaceType != NOT_STRANGER))
        {//识别时间间隔提示陌生人
            this->mIdentifyFaceRecord.time_Start = (double)clock();
#ifdef Q_OS_LINUX
            YNH_LJX::Audio::Audio_PlayPeopleStrangerPcm("zh");
#endif
            emit q_func()->sigTipsMessage(BOTTOM_MESSAGE, 3, QObject::tr("<font color=\"#FFFF33\">stranger</font>"));//陌生人
            //保存记录
            PersonRecordToDB::GetInstance()->appRecordData(this->mIdentifyFaceRecord);
        }
        //检测开门方式
        if(this->CheckDoorOpenMode(this->mIdentifyFaceRecord))
        {//可以通行了
            this->mIdentifyFaceRecord.face.enFaceType = CORE_FACE_RECT_TYPE_POST;
            this->mIdentifyFaceRecord.time_Start = (double)clock();
            emit q_func()->sigPostFaceInfo(this->mCurTemperatureValue, this->mIdentifyFaceRecord.face.attr_info.face_mask);
        }
    }break;
    case CORE_FACE_RECT_TYPE_POST:
        emit q_func()->sigTipsMessage(TOP_MESSAGE, 1, QObject::tr("inqueryHealthCode"));//正在查询健康码
        break;
    case CORE_FACE_RECT_TYPE_DELETE:
    {
        int passtimer = ((double)clock()- this->mIdentifyFaceRecord.time_End)/1000/1000;
        if(this->mIdentifyInterval &&  (passtimer > this->mIdentifyInterval))
        {//超时重新开始实别
#ifdef Q_OS_LINUX
            if(this->mIdentifyFaceRecord.FaceType != NOT_STRANGER)YNH_LJX::Audio::Audio_PlayPeopleStrangerPcm("zh");
#endif
            this->mIdentifyFaceRecord = {};
            this->mIdentityCard = {};//身份证信息
            this->mHealthCode = {};//健康码信息
            this->micCard = {};//ic卡
        }
    }
        break;//到了这里人员已实别所有通过了
    }
    return true;
}
