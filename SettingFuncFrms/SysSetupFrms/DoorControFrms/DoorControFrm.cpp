#include "DoorControFrm.h"
#include "SettingFuncFrms/SetupItemDelegate/CItemWidget.h"
#include "SettingFuncFrms/SetupItemDelegate/CInputBaseDialog.h"

#include "AccessTypeFrm.h"
#include "WigginsOutputFrm.h"
#include "Config/ReadConfig.h"
#include "Application/FaceApp.h"
#include "FaceMainFrm.h"

#include <QListWidget>
#include <QHBoxLayout>
#include <QApplication>
#include <QScreen>
#include <QDesktopWidget>

class DoorControFrmPrivate
{
    Q_DECLARE_PUBLIC(DoorControFrm)
public:
    DoorControFrmPrivate(DoorControFrm *dd);
private:
    void InitUI();
    void InitData();
    void InitConnect();
private:
    QListWidget *m_pListWidget;
private:
    DoorControFrm *const q_ptr;
};

DoorControFrmPrivate::DoorControFrmPrivate(DoorControFrm *dd)
    : q_ptr(dd)
{
    this->InitUI();
    this->InitData();
    this->InitConnect();
}

DoorControFrm::DoorControFrm(QWidget *parent)
    : SettingBaseFrm(parent)
    , d_ptr(new DoorControFrmPrivate(this))
{

}

DoorControFrm::~DoorControFrm()
{

}

void DoorControFrmPrivate::InitUI()
{
    m_pListWidget = new QListWidget;
    QHBoxLayout *layout = new QHBoxLayout(q_func());
    layout->setSpacing(0);
    layout->setMargin(0);
    layout->addWidget(m_pListWidget);
}

void DoorControFrmPrivate::InitData()
{
    int width = QApplication::desktop()->screenGeometry().width();
    int spacing = 0;
    switch(width)
    {
    case 600:spacing = 35;break;
    case 720:spacing = 45;break;
    case 800:spacing = 0;break;
    }

    m_pListWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    {
        QListWidgetItem *pItem = new QListWidgetItem;
        CItemWidget *pItemWidget = new CItemWidget;
        pItemWidget->setAddSpacing(spacing);

        QString accessType;
        switch(ReadConfig::GetInstance()->getDoor_Type()) {
            case 0: accessType = QObject::tr("GateEntry"); break;
            case 1: accessType = QObject::tr("ExitGate"); break;
            case 2: accessType = QObject::tr("EntryExitGate"); break;
            default: accessType = QObject::tr("GateEntry"); break;
        }
        pItemWidget->setData(QObject::tr("AccessType"), ":/Images/SmallRound.png", accessType);
        m_pListWidget->addItem(pItem);
        if (width==480)
            pItemWidget->setAddSpacing(140);			
        m_pListWidget->setItemWidget(pItem, pItemWidget);
    }

    {
        QListWidgetItem *pItem = new QListWidgetItem;
        CItemWidget *pItemWidget = new CItemWidget;
        pItemWidget->setAddSpacing(spacing);

        pItemWidget->setData(QObject::tr("PassagePeriod"), ":/Images/SmallRound.png");//通行时段
        m_pListWidget->addItem(pItem);
        if (width==480)
            pItemWidget->setAddSpacing(140);			
        m_pListWidget->setItemWidget(pItem, pItemWidget);
    }
    {
        QListWidgetItem *pItem = new QListWidgetItem;
        CItemWidget *pItemWidget = new CItemWidget;
        pItemWidget->setAddSpacing(spacing);

        pItemWidget->setData(QObject::tr("WiganFormat"), ":/Images/SmallRound.png", QObject::tr("26bit"));//韦根输出格式
        m_pListWidget->addItem(pItem);
        if (width==480)
            pItemWidget->setAddSpacing(140);			
        m_pListWidget->setItemWidget(pItem, pItemWidget);
    }
    {
        QListWidgetItem *pItem = new QListWidgetItem;
        CItemWidget *pItemWidget = new CItemWidget;
        pItemWidget->setAddSpacing(spacing);

        pItemWidget->setData(QObject::tr("DoorOpeningDelay (relay)"), ":/Images/SmallRound.png", QObject::tr("1s"));//开门延时(继电器)
        m_pListWidget->addItem(pItem);
        if (width==480)
            pItemWidget->setAddSpacing(140);			
        m_pListWidget->setItemWidget(pItem, pItemWidget);
    }

    {
        QListWidgetItem *pItem = new QListWidgetItem;
        CItemWidget *pItemWidget = new CItemWidget;
        pItemWidget->setAddSpacing(spacing);

        pItemWidget->setData(QObject::tr("DoorOpeningMode"), ":/Images/SmallRound.png");//开门方式
        m_pListWidget->addItem(pItem);
        if (width==480)
            pItemWidget->setAddSpacing(140);			
        m_pListWidget->setItemWidget(pItem, pItemWidget);
    }
    {
        QListWidgetItem *pItem = new QListWidgetItem;
        CItemWidget *pItemWidget = new CItemWidget;
        pItemWidget->setAddSpacing(spacing);

        pItemWidget->setData(QObject::tr("DoorPassword"), ":/Images/SmallRound.png");//开门密码
        m_pListWidget->addItem(pItem);
        if (width==480)
            pItemWidget->setAddSpacing(140);			
        m_pListWidget->setItemWidget(pItem, pItemWidget);
    }    
    {
        QListWidgetItem *pItem = new QListWidgetItem;
        CItemWidget *pItemWidget = new CItemWidget;
        pItemWidget->setAddSpacing(spacing);

        pItemWidget->setData(QObject::tr("ScreenOutDelay"), ":/Images/SmallRound.png");//熄屏时间Screen out Delay
        m_pListWidget->addItem(pItem);
        if (width==480)
            pItemWidget->setAddSpacing(140);			
        m_pListWidget->setItemWidget(pItem, pItemWidget);
    }        
}

void DoorControFrmPrivate::InitConnect()
{
    QObject::connect(m_pListWidget, &QListWidget::itemClicked, q_func(), &DoorControFrm::slotIemClicked);
}

void DoorControFrm::setEnter()
{
    Q_D(DoorControFrm);
    QString doorType;
    switch(ReadConfig::GetInstance()->getDoor_Type()) {
        case 0: doorType = QObject::tr("GateEntry"); break;
        case 1: doorType = QObject::tr("ExitGate"); break;
        case 2: doorType = QObject::tr("EntryExitGate"); break;
        default: doorType = QObject::tr("GateEntry"); break;
    }
    ((CItemWidget*)d->m_pListWidget->itemWidget(d->m_pListWidget->item(0)))->setRNameText(doorType);
    ((CItemWidget*)d->m_pListWidget->itemWidget(d->m_pListWidget->item(2)))->setRNameText(ReadConfig::GetInstance()->getDoor_Wiggins() ? QObject::tr("34bit") : QObject::tr("26bit"));
    ((CItemWidget*)d->m_pListWidget->itemWidget(d->m_pListWidget->item(3)))->setRNameText(QString("%1s").arg(ReadConfig::GetInstance()->getDoor_Timer()));
    ((CItemWidget*)d->m_pListWidget->itemWidget(d->m_pListWidget->item(5)))->setRNameText(ReadConfig::GetInstance()->getDoor_Password());
    ((CItemWidget*)d->m_pListWidget->itemWidget(d->m_pListWidget->item(6)))->setRNameText(QString("%1s").arg(ReadConfig::GetInstance()->getScreenOutDelay_Value()));
}

void DoorControFrm::slotIemClicked(QListWidgetItem *item)
{
    Q_D(DoorControFrm);
#ifdef SCREENCAPTURE  //ScreenCapture     
    grab().save(QString("/mnt/user/screenshot/%1.png").arg(this->metaObject()->className()),"png");    
#endif     
    if(item == d->m_pListWidget->item(0))
    {//出入类型
        AccessTypeFrm dlg(this);
        dlg.setAccessType(ReadConfig::GetInstance()->getDoor_Type());
        if(!dlg.exec()) {
            ReadConfig::GetInstance()->setDoor_Type(dlg.getAccessType());
            setEnter(); // Refresh display after change
        }
    }
    else if(item == d->m_pListWidget->item(2))
    {//韦根输出
        WigginsOutputFrm dlg(this);
        dlg.setWigginsOutputType(ReadConfig::GetInstance()->getDoor_Wiggins());
        if(!dlg.exec())ReadConfig::GetInstance()->setDoor_Wiggins(dlg.getWigginsOutputType());
    }
    else if(item == d->m_pListWidget->item(3))
    {//门磁延时
        CInputBaseDialog dlg(this);
        dlg.setTitleText(QObject::tr("DoorOpeningDelay"));//开门延时(继电器)
        dlg.setPlaceholderText("0~60(s)");
        dlg.setIntValidator(0, 60);
        dlg.show();
        if(dlg.exec() == 0)
        {
            QString data = dlg.getData();
            ReadConfig::GetInstance()->setDoor_Timer(data.toInt());
            //回写本地配置表
            ((CItemWidget *)d->m_pListWidget->itemWidget(item))->setRNameText(data + "s");
        }
    }
    else if(item == d->m_pListWidget->item(5))
    {//开门密码
        CInputBaseDialog dlg(this);
        dlg.setTitleText(QObject::tr("DoorPassword"));//开门密码
        dlg.setIntValidator(0, 999);

        QString oldpwd = ReadConfig::GetInstance()->getDoor_Password();
        if (oldpwd=="")
            dlg.setPlaceholderText("000~999");
        else 
            dlg.setData(oldpwd);
        dlg.setIntValidator(0, 999);        
        dlg.show();
        if(dlg.exec() == 0)
        {
            QString data = dlg.getData();
            int idata= data.toInt();
            if (idata==0)
                data = "";//可以清空 000
            else if (idata<10)
                data ="00"+QString::number(idata);
            else if (idata<100)
                data ="0"+QString::number(idata);
            
            printf(">>>%s,%s,%d,data=%s\n",__FILE__,__func__,__LINE__,data.toStdString().data());
            //回写本地配置表
            ReadConfig::GetInstance()->setDoor_Password(data);
            qXLApp->GetFaceMainFrm()->setHomeDisplay_DoorLock(!data.isEmpty());    
            //刷新界面
            ((CItemWidget*)d->m_pListWidget->itemWidget(d->m_pListWidget->item(5)))->setRNameText(data);            
        }
    }
    else if(item == d->m_pListWidget->item(6))
    {//熄屏设置
        CInputBaseDialog dlg(this);
        dlg.setTitleText(QObject::tr("ScreenOutDelay"));//开门密码        
        dlg.setPlaceholderText("0~1800(s)");
        dlg.setIntValidator(0, 1800);        
        dlg.show();        
        if(dlg.exec() == 0)
        {
            QString data = dlg.getData();
            ReadConfig::GetInstance()->setScreenOutDelay_Value(data.toInt());
            //回写本地配置表
            ((CItemWidget *)d->m_pListWidget->itemWidget(item))->setRNameText(data + "s");             
        }
    }
    else emit sigShowFrm(((CItemWidget *)d->m_pListWidget->itemWidget(item))->getNameText());
}

#ifdef SCREENCAPTURE  //ScreenCapture 
void DoorControFrm::mouseDoubleClickEvent(QMouseEvent* event)
{
    grab().save(QString("/mnt/user/screenshot/%1.png").arg(this->metaObject()->className()),"png");    
}
#endif
