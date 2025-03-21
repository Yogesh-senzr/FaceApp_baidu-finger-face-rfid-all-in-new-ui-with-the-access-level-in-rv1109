#include "SysSetupFrm.h"

#include "../SetupItemDelegate/CItemWidget.h"
#include "LanguageFrm.h"
#include "FillLightFrm.h"
#include "LuminanceFrm.h"
#include "VolumeFrm.h"
#include "LogPasswordChangeFrm.h"
#include "QRCodeFrm.h"
#include "SyncFunctionality.h"
#include "Helper/myhelper.h"

#include "Config/ReadConfig.h"
#include "OperationTipsFrm.h"
#include "DeviceInfo.h"

#include <QListWidget>
#include <QHBoxLayout>
#include <QApplication>
#include <QScreen>
#include <QDesktopWidget>
#include <QMouseEvent>
#include <QDebug>
#include <QListWidgetItem>
#include <QtConcurrent/QtConcurrent>
#include <QNetworkInterface>

class SysSetupFrmPrivate
{
    Q_DECLARE_PUBLIC(SysSetupFrm)
public:
    SysSetupFrmPrivate(SysSetupFrm *dd);
private:
    void InitUI();
    void InitData();
    void InitConnect();
private:
    //检测顶层窗口是否显示
    bool CheckTopState();
private:
    QListWidget *m_pListWidget;
private:
    LanguageFrm *m_pLanguageFrm;//语音设置
    LuminanceFrm *m_pLuminanceFrm;//亮度设置
    FillLightFrm *m_pFillLightFrm;//补光设置
    VolumeFrm *m_pVolumeFrm;//音量设置
    LogPasswordChangeFrm *m_pLogPasswordChangeFrm;//登陆密码修改
    QRCodeFrm *m_pQRCodeFrm;
private:
    SysSetupFrm *const q_ptr;
};

SysSetupFrmPrivate::SysSetupFrmPrivate(SysSetupFrm *dd)
    : q_ptr(dd)
{
    m_pQRCodeFrm = new QRCodeFrm(q_ptr); 
    this->InitUI();
    this->InitData();
    this->InitConnect();
}

SysSetupFrm::SysSetupFrm(QWidget *parent)
    : SettingBaseFrm(parent)
    , d_ptr(new SysSetupFrmPrivate(this))
{
    Q_D(SysSetupFrm);
    d->m_pLanguageFrm = new LanguageFrm(this);//语音设置
    d->m_pLuminanceFrm = new LuminanceFrm(this);//亮度设置
    d->m_pFillLightFrm = new FillLightFrm(this);//补光设置
    d->m_pVolumeFrm = new VolumeFrm(this);
    d->m_pLogPasswordChangeFrm = new LogPasswordChangeFrm(this);
    d->m_pQRCodeFrm = new QRCodeFrm(this);
}

SysSetupFrm::~SysSetupFrm()
{

}

void SysSetupFrmPrivate::InitUI()
{
    m_pListWidget = new QListWidget;
    QHBoxLayout *layout = new QHBoxLayout(q_func());
    layout->setSpacing(0);
    layout->setMargin(0);
    layout->addWidget(m_pListWidget);
}

void SysSetupFrmPrivate::InitData()
{
    int width = QApplication::desktop()->screenGeometry().width();
    int spacing = 0;
    switch(width)
    {
    case 480:spacing = 22; break;
    case 600:spacing = 22;break;
    case 720:spacing = 36;break;
    case 800:spacing = 0;break;
    }

    m_pListWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    QStringList listName;

    listName<<QObject::tr("AccessControlSettings")<<QObject::tr("MainUISettings")<<QObject::tr("TimeSetting")
    <<QObject::tr("StorageCapacity")<<QObject::tr("LoginPassword")<<QObject::tr("About");

    {   // Language Settings
        QListWidgetItem *pItem = new QListWidgetItem;
        CItemWidget *pItemWidget = new CItemWidget;
        pItemWidget->setAddSpacing(spacing);
        pItemWidget->setData(QObject::tr("LanguageSettings"), ":/Images/SmallRound.png", QObject::tr("English"));
        m_pListWidget->addItem(pItem);
        if (width==480)
            pItemWidget->setAddSpacing(120);
        m_pListWidget->setItemWidget(pItem, pItemWidget);
    }

    {   // Brightness Settings
        QListWidgetItem *pItem = new QListWidgetItem;
        CItemWidget *pItemWidget = new CItemWidget;
        pItemWidget->setAddSpacing(spacing);
        pItemWidget->setData(QObject::tr("BrightnessSetting"), ":/Images/SmallRound.png", "10");
        m_pListWidget->addItem(pItem);
        if (width==480)
            pItemWidget->setAddSpacing(120);
        m_pListWidget->setItemWidget(pItem, pItemWidget);
    }

    {   // Fill Light Settings
        QListWidgetItem *pItem = new QListWidgetItem;
        CItemWidget *pItemWidget = new CItemWidget;
        pItemWidget->setAddSpacing(spacing);
        pItemWidget->setData(QObject::tr("FillLightSetting"), ":/Images/SmallRound.png", QObject::tr("Auto"));
        m_pListWidget->addItem(pItem);
        if (width==480)
            pItemWidget->setAddSpacing(120);
        m_pListWidget->setItemWidget(pItem, pItemWidget);
    }

    {   // Volume Settings
        QListWidgetItem *pItem = new QListWidgetItem;
        CItemWidget *pItemWidget = new CItemWidget;
        pItemWidget->setAddSpacing(spacing);
        pItemWidget->setData(QObject::tr("VolumeSetting"), ":/Images/SmallRound.png", "8");
        m_pListWidget->addItem(pItem);
        if (width==480)
            pItemWidget->setAddSpacing(120);
        m_pListWidget->setItemWidget(pItem, pItemWidget);
    }

    // QR Code item before about
    {
        QListWidgetItem *pItem = new QListWidgetItem;
        CItemWidget *pItemWidget = new CItemWidget;
        pItemWidget->setAddSpacing(spacing);
        pItemWidget->setData(QObject::tr("QR Enrollment"), ":/Images/SmallRound.png");
        m_pListWidget->addItem(pItem);
        if (width==480)
            pItemWidget->setAddSpacing(120);
        m_pListWidget->setItemWidget(pItem, pItemWidget);
    }

    // Add the rest of the items from listName
    for(int i = 0; i < listName.count(); i++)
    {
        QListWidgetItem *pItem = new QListWidgetItem;
        CItemWidget *pItemWidget = new CItemWidget;
        pItemWidget->setAddSpacing(spacing);
        pItemWidget->setData(listName.at(i), ":/Images/SmallRound.png");
        m_pListWidget->addItem(pItem);
        if (width==480)
            pItemWidget->setAddSpacing(120);
        m_pListWidget->setItemWidget(pItem, pItemWidget);
    }

    {   // Sync Devices
        QListWidgetItem *pItem = new QListWidgetItem;
        CItemWidget *pItemWidget = new CItemWidget;
        pItemWidget->setAddSpacing(spacing);
        pItemWidget->setData(QObject::tr("Sync Devices"), ":/Images/SmallRound.png");
        m_pListWidget->addItem(pItem);
        if (width == 480)
            pItemWidget->setAddSpacing(120);
        m_pListWidget->setItemWidget(pItem, pItemWidget);
    }
    
}

void SysSetupFrmPrivate::InitConnect()
{
    QObject::connect(m_pListWidget, &QListWidget::itemClicked, q_func(), &SysSetupFrm::slotIemClicked);
}

bool SysSetupFrmPrivate::CheckTopState()
{
    if(!m_pLuminanceFrm->isHidden())
    {
        ((CItemWidget *)this->m_pListWidget->itemWidget(this->m_pListWidget->item(1)))->setRNameText(QString::number(m_pLuminanceFrm->getAdjustValue()));
        ReadConfig::GetInstance()->setLuminance_Value(m_pLuminanceFrm->getAdjustValue());
        m_pLuminanceFrm->hide();

        return true;
    }else if(!m_pVolumeFrm->isHidden())
    {
        ((CItemWidget *)this->m_pListWidget->itemWidget(this->m_pListWidget->item(3)))->setRNameText(QString::number(m_pVolumeFrm->getAdjustValue()));
        m_pVolumeFrm->hide();
        return true;
    }
    return false;
}

static QString LanguageIdexToSTR(const int &index)
{
    switch(index)
    {
    case 0:return QObject::tr("English");
    case 1:return QObject::tr("English");
    //case 2:return QObject::tr("日本語");
    //case 3:return QObject::tr("한글");
    }
    return QObject::tr("English");
}

static QString FillLightIndexToSTR(const int &index)
{
    switch(index)
    {
        case 0:return QObject::tr("NormallyClosed");//常闭
        case 1:return QObject::tr("NormallyOpen");//常开
        case 2:return QObject::tr("Auto");//自动
    }
    return QObject::tr("Auto");//自动
}

void SysSetupFrm::setEnter()
{
    Q_D(SysSetupFrm);
    //获取语言
    d->m_pLanguageFrm->setLanguageMode(ReadConfig::GetInstance()->getLanguage_Mode());
    ((CItemWidget *)d->m_pListWidget->itemWidget(d->m_pListWidget->item(0)))->setRNameText(LanguageIdexToSTR(ReadConfig::GetInstance()->getLanguage_Mode()));
    //获取亮度
    d->m_pLuminanceFrm->setAdjustValue(ReadConfig::GetInstance()->getLuminance_Value());
    ((CItemWidget *)d->m_pListWidget->itemWidget(d->m_pListWidget->item(1)))->setRNameText(QString::number(ReadConfig::GetInstance()->getLuminance_Value()));
    //获取补光
    d->m_pFillLightFrm->setFillLightMode(ReadConfig::GetInstance()->getFillLight_Value());
    ((CItemWidget *)d->m_pListWidget->itemWidget(d->m_pListWidget->item(2)))->setRNameText(FillLightIndexToSTR(ReadConfig::GetInstance()->getFillLight_Value()));
    //获取音量
    d->m_pVolumeFrm->setAdjustValue(ReadConfig::GetInstance()->getVolume_Value());
    ((CItemWidget *)d->m_pListWidget->itemWidget(d->m_pListWidget->item(3)))->setRNameText(QString::number(ReadConfig::GetInstance()->getVolume_Value()));
}

void SysSetupFrm::setLeaveEvent()
{
    Q_D(SysSetupFrm);
	d->CheckTopState();	//点击离开时保存设置值
    if(!d->m_pLuminanceFrm->isHidden())d->m_pLuminanceFrm->hide();
    if(!d->m_pVolumeFrm->isHidden())d->m_pVolumeFrm->hide();
    QtConcurrent::run(ReadConfig::GetInstance(), &ReadConfig::setSaveConfig);
}

void SysSetupFrm::slotIemClicked(QListWidgetItem *item)
{
    Q_D(SysSetupFrm);
#ifdef SCREENCAPTURE  //ScreenCapture     
    grab().save(QString("/mnt/user/screenshot/%1.png").arg(this->metaObject()->className()),"png");  
#endif 
    int width = QApplication::desktop()->screenGeometry().width();    
    if(d->CheckTopState())
    {//不处理
        return;
    }
    else if(item == d->m_pListWidget->item(0))
    {//语言设置 (Language Settings)
        d->m_pLanguageFrm->exec();
        ReadConfig::GetInstance()->setLanguage_Mode(d->m_pLanguageFrm->getLanguageMode());
        ((CItemWidget *)d->m_pListWidget->itemWidget(d->m_pListWidget->item(0)))->setRNameText(LanguageIdexToSTR(ReadConfig::GetInstance()->getLanguage_Mode()));
    }
    else if(item == d->m_pListWidget->item(1))
    {//亮度设置 (Brightness Settings)
        d->m_pLuminanceFrm->show();
    }
    else if(item == d->m_pListWidget->item(2))
    {//补光设置 (Fill Light Settings)
        d->m_pFillLightFrm->exec();
        ReadConfig::GetInstance()->setFillLight_Value(d->m_pFillLightFrm->getFillLightMode());
        ((CItemWidget *)d->m_pListWidget->itemWidget(d->m_pListWidget->item(2)))->setRNameText(FillLightIndexToSTR(ReadConfig::GetInstance()->getFillLight_Value()));
    }
    else if(item == d->m_pListWidget->item(3))
    {//音量设置 (Volume Settings)
        d->m_pVolumeFrm->show();
    }
    else if(item == d->m_pListWidget->item(4))
    {//QR Code Enrollment
        QString serialNumber = DeviceInfo::getInstance()->getSerialNumber(); // Retrieve serial from DeviceInfo
        d->m_pQRCodeFrm->setSerialNumber();
        d->m_pQRCodeFrm->exec();
    }
    
    else if(item == d->m_pListWidget->item(5))
    {//Access Control Settings
        emit sigShowFrm(((CItemWidget *)d->m_pListWidget->itemWidget(item))->getNameText());
    }
    
    else if(item == d->m_pListWidget->item(6))
    {//Main UI Settings
        emit sigShowFrm(((CItemWidget *)d->m_pListWidget->itemWidget(item))->getNameText());
    }
    else if(item == d->m_pListWidget->item(7))
    {//Time Settings
        emit sigShowFrm(((CItemWidget *)d->m_pListWidget->itemWidget(item))->getNameText());
    }
    else if(item == d->m_pListWidget->item(8))
    {//Storage Capacity
        emit sigShowFrm(((CItemWidget *)d->m_pListWidget->itemWidget(item))->getNameText());
    }
    else if(item == d->m_pListWidget->item(9))
    {//Login Password
        if (width==480)
            d->m_pLogPasswordChangeFrm->setFixedSize(d->m_pLogPasswordChangeFrm->width()-40, d->m_pLogPasswordChangeFrm->height());
        d->m_pLogPasswordChangeFrm->show();
        int ret = d->m_pLogPasswordChangeFrm->exec();
        if(ret == 0)
            ReadConfig::GetInstance()->setLoginPassword(d->m_pLogPasswordChangeFrm->getNewPasw());
    }
    else if(item == d->m_pListWidget->item(10))
    {//About
        emit sigShowFrm(((CItemWidget *)d->m_pListWidget->itemWidget(item))->getNameText());
    }

    else if (item == d->m_pListWidget->item(11)) // Adjust index if necessary
    {
        SyncFunctionality::ShowSyncDialog();
    }

}

#ifdef SCREENCAPTURE  //ScreenCapture 
void SysSetupFrm::mouseDoubleClickEvent(QMouseEvent* event)
{
    grab().save(QString("/mnt/user/screenshot/%1.png").arg(this->metaObject()->className()),"png");    
}
#endif 
