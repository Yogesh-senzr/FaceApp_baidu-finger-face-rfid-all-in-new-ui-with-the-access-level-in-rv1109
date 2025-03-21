#include "ZhangjiakoudentityManagement.h"
#include "IdentityManagementPrivate.h"

#ifdef Q_OS_LINUX
#include "PCIcore/Audio.h"
#include "BaseFace/BaseFaceManager.h"
#include "HealthCodeDevices/HealthCodeManage.h"
#endif
#include "Zhangjiakou/ZhangjiakouHealthCode.h"

#include "DB/RegisteredFacesDB.h"
#include "Application/FaceApp.h"

#include <QDebug>
#include <QDateTime>

class ZhangjiakoudentityManagementPrivate : public IdentityManagementPrivate
{
    Q_DECLARE_PUBLIC(ZhangjiakoudentityManagement)
public:
    ZhangjiakoudentityManagementPrivate(ZhangjiakoudentityManagement *dd);
private://检测开门方式
    bool CheckDoorOpenMode(IdentifyFaceRecord_t &);
private:
    ZhangjiakouHealthCode *m_pZhangjiakouHealthCode;
};

ZhangjiakoudentityManagementPrivate::ZhangjiakoudentityManagementPrivate(ZhangjiakoudentityManagement *dd)
    : IdentityManagementPrivate(dd)
    , m_pZhangjiakouHealthCode(new ZhangjiakouHealthCode)
{
}

ZhangjiakoudentityManagement::ZhangjiakoudentityManagement(QObject *parent)
    : IdentityManagement(new ZhangjiakoudentityManagementPrivate(this), parent)
{
    QObject::connect(d_func()->m_pZhangjiakouHealthCode, &ZhangjiakouHealthCode::sigHealthCodeInfo, this, &ZhangjiakoudentityManagement::slotHealthCodeInfo);
    QObject::connect(this, &ZhangjiakoudentityManagement::sigQueryHealthCode, d_func()->m_pZhangjiakouHealthCode, &ZhangjiakouHealthCode::slotQueryHealthCode);
    QObject::connect(this, &ZhangjiakoudentityManagement::sigPostTempRecognition, d_func()->m_pZhangjiakouHealthCode, &ZhangjiakouHealthCode::slotPostTempRecognition);
}

ZhangjiakoudentityManagement::~ZhangjiakoudentityManagement()
{

}

void ZhangjiakoudentityManagement::slotIdentityCardInfo(const QString name, const QString idCard, const QString sex, const QString path)
{
    Q_UNUSED(path);
    Q_D(ZhangjiakoudentityManagement);
    d->sync.lock();
    d->mIdentityCard = {name, sex, idCard, path, QDateTime::currentDateTime().toTime_t()};
    d->sync.unlock();
    emit sigQueryHealthCode(name, idCard, sex);
}

bool ZhangjiakoudentityManagementPrivate::CheckDoorOpenMode(IdentifyFaceRecord_t &t)
{
    QString door_mode = this->mMustOpenMode;
    QString process = t.process_state;
    if(door_mode.contains(Door_OpenModeToSTR(_DOOR_OPEN_MODE::MASK))
            && (t.face.attr_info.face_mask != 1))
    {//检测口罩
#ifdef Q_OS_LINUX
        YNH_LJX::Audio::Audio_PlayUnbtMaskPcm("zh");
#endif
        emit q_func()->sigTipsMessage(TOP_MESSAGE, 1, QObject::tr("DOOR_OPEN_MODE_MASK_Hint"));//请佩戴口罩

    }else if(door_mode.contains(Door_OpenModeToSTR(_DOOR_OPEN_MODE::THERMOMETRY))
             && (t.temp_value>=this->mAlarmTemp))
    {/*报体温异常*/
#ifdef Q_OS_LINUX
        YNH_LJX::Audio::Audio_PlayTempabnormalPcm("zh");
#endif
        emit q_func()->sigTipsMessage(TOP_MESSAGE, 1, QObject::tr("DOOR_OPEN_MODE_THERMOMETRY_Hint"));//体温异常

    }else if(door_mode.contains(Door_OpenModeToSTR(_DOOR_OPEN_MODE::SWIPING_FACE))
             && process.contains(Door_OpenModeToSTR(_DOOR_OPEN_MODE::SWIPING_FACE))
             && t.FaceType != NOT_STRANGER)
    {//刷脸,已经实别但是不通过
        emit q_func()->sigTipsMessage(BOTTOM_MESSAGE, 3, QObject::tr("<font color=\"#FFFF33\">stranger</font>"));//<font color=\"#FFFF33\">陌生人</font>

    }else if(door_mode.contains(Door_OpenModeToSTR(_DOOR_OPEN_MODE::SWIPING_FACE))
             && !process.contains(Door_OpenModeToSTR(_DOOR_OPEN_MODE::SWIPING_FACE)))
    {//刷脸,已经实别但是没有处理过
        emit q_func()->sigTipsMessage(TOP_MESSAGE, 1, QObject::tr("FaceRecognitioning"));//正在识别人脸

    }else if(door_mode.contains(Door_OpenModeToSTR(_DOOR_OPEN_MODE::ICCARD))
             && !process.contains(Door_OpenModeToSTR(_DOOR_OPEN_MODE::ICCARD)))
    {//提示刷卡
        if(this->micCard.number.isEmpty())
        {//卡号是空的， 提示刷卡
            emit q_func()->sigTipsMessage(TOP_MESSAGE, 1, QObject::tr("Plsswipe"));//请刷卡
        }else if(((t.time_End - this->micCard.timer)/1000/1000)>=20)
        {//卡号不为空,查找一下卡号是否存
            emit q_func()->sigTipsMessage(TOP_MESSAGE, 1, QObject::tr("PlsswipeAgain"));//请重刷卡
        }else
        {//查询卡是否存
            PERSONS_s info;
            if(RegisteredFacesDB::GetInstance()->selectICcardPerson(this->micCard.number, info))
            {
                if(info.iccard == this->micCard.number)
                {
                    t.process_state.append("&1");
                    t.FaceType = NOT_STRANGER;//是否陌生人, 1是陌生人，2注册人员， 0未识别
                    t.face_personid = info.personid;//人脸库id
                    t.face_persontype = info.persontype;//是否陌生人（1陌生人，2非陌生人）
                    /*下面字段是人员录入信息库*/
                    t.face_name = info.name;//名称
                    t.face_sex = info.sex;//性别
                    t.face_uuid = info.uuid;//人脸信息的数据库的标一标识码
                    t.face_idcardnum = info.idcard;//身份证号
                    t.face_iccardnum = this->micCard.number;
                    t.face_gids = info.gids;//组
                    t.face_aids = info.pids;
                    qDebug()<<"读取到ic卡信息:"<<info.name<<" iccard:"<<info.iccard;
                }else emit q_func()->sigTipsMessage(TOP_MESSAGE, 1, QObject::tr("CardNumberNotExist"));//卡号不存在
            }else emit q_func()->sigTipsMessage(TOP_MESSAGE, 1, QObject::tr("ErrorQueryingData"));
        }

    }else if(door_mode.contains(Door_OpenModeToSTR(_DOOR_OPEN_MODE::QRCODE))
             && !process.contains(Door_OpenModeToSTR(_DOOR_OPEN_MODE::QRCODE)))
    {//提示二唯码
        if(this->mHealthCode.timer == 0)
        {//未刷过二维码
            emit q_func()->sigTipsMessage(TOP_MESSAGE, 1, QObject::tr("PlsswipeNHcode"));//请刷国康码
        }else if(((t.time_End - this->mHealthCode.timer)/1000/1000)>=30)
        {
            emit q_func()->sigTipsMessage(TOP_MESSAGE, 1, QObject::tr("PlsswipeNHcodeAgain"));//请重刷国康码
        }else if(this->mHealthCode.qrcode == 0)
        {
            t.process_state.append("&5");
        }

    }else if(door_mode.contains(Door_OpenModeToSTR(_DOOR_OPEN_MODE::IDCARD))
             && !process.contains(Door_OpenModeToSTR(_DOOR_OPEN_MODE::IDCARD)))
    {//提示身份证
        if(this->mIdentityCard.idCard.isEmpty())
        {//卡号是空的， 提示刷卡
            emit q_func()->sigTipsMessage(TOP_MESSAGE, 1, QObject::tr("PlsswipeIdCard"));//请刷身份证
        }else if(((t.time_End - this->mIdentityCard.timer)/1000/1000)>=30)
        {//卡号不为空,查找一下卡号是否存
            emit q_func()->sigTipsMessage(TOP_MESSAGE, 1, QObject::tr("PlsswipeIdCardAgain"));//请重刷身份证
        }else
        {//查找身份
            PERSONS_s info;
            if(RegisteredFacesDB::GetInstance()->selectIDcardPerson(this->mIdentityCard.idCard, info))
            {
                if(info.idcard == this->mIdentityCard.idCard)
                {
                    t.process_state.append("&6");
                    t.FaceType = NOT_STRANGER;//是否陌生人, 1是陌生人，2注册人员， 0未识别
                    t.face_personid = info.personid;//人脸库id
                    t.face_persontype = info.persontype;//是否陌生人（1陌生人，2非陌生人）
                    /*下面字段是人员录入信息库*/
                    t.face_name = info.name;//名称
                    t.face_sex = info.sex;//性别
                    t.face_uuid = info.uuid;//人脸信息的数据库的标一标识码
                    t.face_idcardnum = info.idcard;//身份证号
                    t.face_iccardnum = info.iccard;
                    t.face_gids = info.gids;//组
                    t.face_aids = info.pids;
                }
                else emit q_func()->sigTipsMessage(TOP_MESSAGE, 1, QObject::tr("IdCardNoNotExist"));//身份证不存在
            }else emit q_func()->sigTipsMessage(TOP_MESSAGE, 1, QObject::tr("ErrorQueryingData"));//查询数据出错
        }
    }else if(door_mode.contains(Door_OpenModeToSTR(_DOOR_OPEN_MODE::PERSON_IDCARD))
             && !process.contains(Door_OpenModeToSTR(_DOOR_OPEN_MODE::PERSON_IDCARD)))
    {//提示人证比对
        if(this->mIdentityCard.idCard.isEmpty())
        {
            emit q_func()->sigTipsMessage(TOP_MESSAGE, 1, QObject::tr("PlsswipeIdCard"));//请刷身份证
        }else if(((t.time_End - this->mIdentityCard.timer)/1000/1000)>=30)
        {
            emit q_func()->sigTipsMessage(TOP_MESSAGE, 1, QObject::tr("PlsswipeIdCardAgain"));//请重刷身份证
        }else
        {//进行人脸比对
            double similar = 0.0;
#ifdef Q_OS_LINUX
            similar = qXLApp->GetAlgoFaceManager()->getPersonIdCardCompare(this->mIdentityCard.pathImage);
#endif
            //qDebug()<<__FILE__<< __FUNCTION__<<__LINE__<<"similar "<<similar <<"    "<<this->mHealthCode.type << " "<<this->mHealthCode.qrcode;
            if(similar>=this->midcardThanthreshold)
            {
                t.FaceType = NOT_STRANGER;
                if(!this->mHealthCode.idCard.isEmpty() && (this->mHealthCode.type == 2))
                {
                    emit q_func()->sigTipsMessage(TOP_MESSAGE, 1, this->mHealthCode.msg);
                    if(this->mHealthCode.qrcode == 0)t.process_state.append("&7");
                    //上传温度
                    if(this->mHealthCode.qrcode>=0 && this->mHealthCode.qrcode<=2)
                    {
                        bool warning = false;
                        if((this->mHealthCode.qrcode) || (this->mCurTemperatureValue >= this->mHealthCode.warningTemp))warning = true;
                        emit q_func()->sigPostTempRecognition("/isc/TempPersonBlast.jpeg", this->mIdentityCard.name, this->mIdentityCard.idCard, this->mIdentityCard.sex,
                                                              2, 1, this->mHealthCode.qrcode, this->mCurTemperatureValue, similar, warning);

                    }else if(this->mCurTemperatureValue >= this->mHealthCode.warningTemp)
                    {//健康码获取异常, 体温不正常
                        emit q_func()->sigPostTempRecognition("/isc/TempPersonBlast.jpeg", this->mIdentityCard.name, this->mIdentityCard.idCard, this->mIdentityCard.sex,
                                                              2, 1, this->mHealthCode.qrcode, this->mCurTemperatureValue, similar, true);
                    }
                }else emit q_func()->sigTipsMessage(TOP_MESSAGE, 1, QObject::tr("GettingHealthCode"));//正在获取健康码 Getting health code
            }else emit q_func()->sigTipsMessage(TOP_MESSAGE, 1, QObject::tr("PersonIdCardCompareBad"));//非本人证件
        }
    }else return true;
    return false;
}
