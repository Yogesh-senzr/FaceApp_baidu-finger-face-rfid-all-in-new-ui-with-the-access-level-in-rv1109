#include "AboutMachineFrm.h"
#include "../SetupItemDelegate/CItemWidget.h"
#include "OperationTipsFrm.h"
#include "SystemMaintenanceFrm.h"
#include "DevicePoweroffFrm.h"

#include "Config/ReadConfig.h"
#include "Helper/myhelper.h"
#include "Version.h"
#include "../SetupItemDelegate/CItemBoxWidget.h"

#include "../SettingEditTextFrm.h"

#include <QDebug>
#include <QListWidget>
#include <QHBoxLayout>
#include <QFile>
#include <QApplication>
#include <QScreen>
#include <QDesktopWidget>
#include <QtConcurrent/QtConcurrent>

class AboutMachineFrmPrivate
{
    Q_DECLARE_PUBLIC(AboutMachineFrm)
public:
    AboutMachineFrmPrivate(AboutMachineFrm *dd);
private:
    void InitUI();
    void InitData();
    void InitConnect();
    bool CheckTopWidget();
private:
    QListWidget *m_pListWidget;
private:
    AboutMachineFrm *const q_ptr;
};

AboutMachineFrmPrivate::AboutMachineFrmPrivate(AboutMachineFrm *dd)
    : q_ptr(dd)
{
    this->InitUI();
    this->InitData();
    this->InitConnect();
}

AboutMachineFrm::AboutMachineFrm(QWidget *parent)
    : SettingBaseFrm(parent)
    , d_ptr(new AboutMachineFrmPrivate(this))
{

}

AboutMachineFrm::~AboutMachineFrm()
{

}

void AboutMachineFrmPrivate::InitUI()
{
    m_pListWidget = new QListWidget;
    QHBoxLayout *layout = new QHBoxLayout(q_func());
    layout->setSpacing(0);
    layout->setMargin(0);
    layout->addWidget(m_pListWidget);
}

bool AboutMachineFrmPrivate::CheckTopWidget()
{
    foreach (QWidget *w, qApp->topLevelWidgets()) {
        if(w->objectName() == "InputBaseDialog")return true;
    }
    return false;
}

void AboutMachineFrmPrivate::InitData()
{
    int width = QApplication::desktop()->screenGeometry().width();
    int spacing = 0;
    switch(width)
    {
    case 480:spacing = 22;break;		
    case 600:spacing = 22;break;
    case 720:spacing = 32;break;
    case 800:spacing = 0;break;
    }

    m_pListWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    {
        QListWidgetItem *pItem = new QListWidgetItem;
        CItemWidget *pItemWidget = new CItemWidget;
        pItemWidget->setAddSpacing(spacing);
        pItemWidget->setAddSpacing(12);
        //pItemWidget->setData(QObject::tr("Verson"), ":/Images/SmallRound.png", QObject::tr("V2.1-20211115"));//系统版本 //ISC_VERSION
        pItemWidget->setData(QObject::tr("Verson"), ":/Images/SmallRound.png", ISC_VERSION);//系统版本 //
        pItemWidget->setHideRPng();
        m_pListWidget->addItem(pItem);
		if (width ==480)
			pItemWidget->setAddSpacing(120);			
        m_pListWidget->setItemWidget(pItem, pItemWidget);
    } 
    {
        QListWidgetItem *pItem = new QListWidgetItem;
        CItemWidget *pItemWidget = new CItemWidget;
        pItemWidget->setAddSpacing(spacing);
        pItemWidget->setAddSpacing(12);
        pItemWidget->setData(QObject::tr("SerianNo"), ":/Images/SmallRound.png");//SN序列号
        pItemWidget->setHideRPng();
        m_pListWidget->addItem(pItem);
		if (width ==480)
			pItemWidget->setAddSpacing(120);			
        m_pListWidget->setItemWidget(pItem, pItemWidget);
    }
    {
        QListWidgetItem *pItem = new QListWidgetItem;
        CItemWidget *pItemWidget = new CItemWidget;
        pItemWidget->setAddSpacing(spacing);
        pItemWidget->setAddSpacing(12);
        pItemWidget->setData(QObject::tr("algoVersion"), ":/Images/SmallRound.png");//算法版本
        pItemWidget->setHideRPng();
        m_pListWidget->addItem(pItem);
		if (width ==480)
			pItemWidget->setAddSpacing(120);			
        m_pListWidget->setItemWidget(pItem, pItemWidget);
    }
    {    
        QListWidgetItem *pItem = new QListWidgetItem;
        CItemBoxWidget *pItemWidget = new CItemBoxWidget;
        pItemWidget->setAddSpacing(spacing);
        pItemWidget->setAddSpacing(12);
        pItemWidget->setData(QObject::tr("debugMode"), ":/Images/SmallRound.png");//开发者模式
        m_pListWidget->addItem(pItem);
		if (width ==480)
			pItemWidget->setAddSpacing(120);			
        m_pListWidget->setItemWidget(pItem, pItemWidget);
    }       
    {
        QListWidgetItem *pItem = new QListWidgetItem;
        CItemWidget *pItemWidget = new CItemWidget;
        pItemWidget->setAddSpacing(spacing);
        pItemWidget->setData(QObject::tr("ScreenParam"), ":/Images/SmallRound.png");//屏参文件
        m_pListWidget->addItem(pItem);
		if (width ==480)
			pItemWidget->setAddSpacing(120);			
        m_pListWidget->setItemWidget(pItem, pItemWidget);
    }    
    {
        QListWidgetItem *pItem = new QListWidgetItem;
        CItemWidget *pItemWidget = new CItemWidget;
        pItemWidget->setAddSpacing(spacing);
        pItemWidget->setData(QObject::tr("TouchScreen"), ":/Images/SmallRound.png");//触摸屏文件
        m_pListWidget->addItem(pItem);
		if (width ==480)
			pItemWidget->setAddSpacing(120);			
        m_pListWidget->setItemWidget(pItem, pItemWidget);
    }     
    {
        QListWidgetItem *pItem = new QListWidgetItem;
        CItemWidget *pItemWidget = new CItemWidget;
        pItemWidget->setAddSpacing(spacing);
        pItemWidget->setData(QObject::tr("SystemUpgrade"), ":/Images/SmallRound.png");//系统升级
        m_pListWidget->addItem(pItem);
		if (width ==480)
			pItemWidget->setAddSpacing(120);			
        m_pListWidget->setItemWidget(pItem, pItemWidget);
    }
    {
        QListWidgetItem *pItem = new QListWidgetItem;
        CItemWidget *pItemWidget = new CItemWidget;
        pItemWidget->setAddSpacing(spacing);
        pItemWidget->setData(QObject::tr("SystemMaintenance"), ":/Images/SmallRound.png");//系统维护
        m_pListWidget->addItem(pItem);
		if (width ==480)
			pItemWidget->setAddSpacing(120);			
        m_pListWidget->setItemWidget(pItem, pItemWidget);
    }
    {
        QListWidgetItem *pItem = new QListWidgetItem;
        CItemWidget *pItemWidget = new CItemWidget;
        pItemWidget->setAddSpacing(spacing);
        pItemWidget->setData(QObject::tr("Shutdown"), ":/Images/SmallRound.png");//设备关机
        m_pListWidget->addItem(pItem);
		if (width ==480)
			pItemWidget->setAddSpacing(120);			
        m_pListWidget->setItemWidget(pItem, pItemWidget);
    }
    {
        QListWidgetItem *pItem = new QListWidgetItem;
        CItemWidget *pItemWidget = new CItemWidget;
        pItemWidget->setAddSpacing(spacing);
        pItemWidget->setData(QObject::tr("ReturnSetting"), ":/Images/SmallRound.png");//还原所有设置
        m_pListWidget->addItem(pItem);
		if (width ==480)
			pItemWidget->setAddSpacing(120);			
        m_pListWidget->setItemWidget(pItem, pItemWidget);
    }
    {
        QListWidgetItem *pItem = new QListWidgetItem;
        CItemWidget *pItemWidget = new CItemWidget;
        pItemWidget->setAddSpacing(spacing);
        pItemWidget->setData(QObject::tr("ReturnToFactorySetting"), ":/Images/SmallRound.png");//恢复出厂设置
        m_pListWidget->addItem(pItem);
		if (width ==480)
			pItemWidget->setAddSpacing(120);			
        m_pListWidget->setItemWidget(pItem, pItemWidget);
    }
}

void AboutMachineFrmPrivate::InitConnect()
{
    QObject::connect(m_pListWidget, &QListWidget::itemClicked, q_func(), &AboutMachineFrm::slotIemClicked);
}

void AboutMachineFrm::setEnter()
{//进入时
    Q_D(AboutMachineFrm);
    ((CItemWidget *)d->m_pListWidget->itemWidget(d->m_pListWidget->item(1)))->setRNameText(myHelper::getCpuSerial());
    ((CItemWidget *)d->m_pListWidget->itemWidget(d->m_pListWidget->item(2)))->setRNameText(QObject::tr("Baidu_ver_2.1.119"));
    ((CItemBoxWidget *)d->m_pListWidget->itemWidget(d->m_pListWidget->item(3)))->setCheckBoxState(ReadConfig::GetInstance()->getDebugMode_Value());    
}

void AboutMachineFrm::setLeaveEvent()
{
    Q_D(AboutMachineFrm);
    //活体检测
    //ReadConfig::GetInstance()->setIdentity_Manager_CheckLiving(((CItemBoxWidget *)d->m_pListWidget->itemWidget(d->m_pListWidget->item(0)))->getCheckBoxState() ? 1 : 0);
        printf(">>>%s,%s,%d,getCheckBoxState=%d\n",__FILE__ , __func__,__LINE__, ((CItemBoxWidget *)d->m_pListWidget->itemWidget(d->m_pListWidget->item(3)))->getCheckBoxState());
        ReadConfig::GetInstance()->setDebugMode_Value(((CItemBoxWidget *)d->m_pListWidget->itemWidget(d->m_pListWidget->item(3)))->getCheckBoxState() ? 1 : 0);    
    foreach (QWidget *w, qApp->topLevelWidgets()) {
        if(w->objectName() == "InputBaseDialog")
        {//如果存在顶层用户未关闭的窗口， 使其不可见
            QDialog *dlg = (QDialog *)w;
            dlg->done(1);
        }
    }

    QtConcurrent::run(ReadConfig::GetInstance(), &ReadConfig::setSaveConfig);
}

void AboutMachineFrm::slotIemClicked(QListWidgetItem *item)
{
    Q_D(AboutMachineFrm);
#ifdef SCREENCAPTURE  //ScreenCapture     
    grab().save(QString("/mnt/user/screenshot/%1.png").arg(this->metaObject()->className()),"png");    
#endif 
    if(item == d->m_pListWidget->item(3))
    {//开发者模式　

        printf(">>>%s,%s,%d,getCheckBoxState=%d\n",__FILE__ , __func__,__LINE__, ((CItemBoxWidget *)d->m_pListWidget->itemWidget(d->m_pListWidget->item(3)))->getCheckBoxState());
        ReadConfig::GetInstance()->setDebugMode_Value(((CItemBoxWidget *)d->m_pListWidget->itemWidget(d->m_pListWidget->item(3)))->getCheckBoxState() ? 1 : 0);
        //ReadConfig::GetInstance()->setIdentity_Manager_CheckLiving(((CItemBoxWidget *)d->m_pListWidget->itemWidget(d->m_pListWidget->item(0)))->getCheckBoxState() ? 1 : 0);
    }
    else if(item == d->m_pListWidget->item(4))
    {//屏参
        //OperationTipsFrm dlg(this);
        //系统升级,确定,请索取update.img升级包文件放入U盘(FAT32)，插入U盘即可自动升级!
        //dlg.setMessageBox(QObject::tr("SystemUpgrade"), QObject::tr("SystemUpgradeHint"), QObject::tr("Ok"), "", 1);
        //SettingMenuTitleFrm *pItemWidget = new SettingMenuTitleFrm;   
        //SettingEditTextFrm *pItemWidget = new SettingEditTextFrm;
        SettingEditTextFrm dlg(this);
        dlg.setTitle(QObject::tr("TouchScreen"));
        QFile file("/param/RV1109_PARAM.txt");        
        if (file.open(QIODevice::ReadOnly))
        {
            QByteArray array;
            while (file.atEnd() == false)
            {
                array += file.readLine();
            }
            dlg.setData(array);
        }        

		dlg.move((QApplication::desktop()->screenGeometry().width()-dlg.width())/2, (QApplication::desktop()->screenGeometry().height()-dlg.height())/2);
        dlg.exec();

    }    
    else if(item == d->m_pListWidget->item(5))
    {//触摸屏参数
        SettingEditTextFrm dlg(this);
        dlg.setTitle(QObject::tr("TouchScreen"));
        //cat /proc/gt9xx_config
        QString str = "";
        FILE *pFile = popen("cat /proc/gt9xx_config", "r");
        if (pFile)
        {
        	std::string ret = "";
    		char buf[3072-32-64-436-8] = { 0 };
    		int readSize = 0;
    		do
    		{
    			readSize = fread(buf, 1, sizeof(buf), pFile);
    			if (readSize > 0)
    			{
    				ret += std::string(buf, 0, readSize);
    			}
    		} //while (readSize > 0);
            while (0);
            pclose(pFile);

            str = QString::fromStdString(ret);
            dlg.setData(str);          
        }
		dlg.move((QApplication::desktop()->screenGeometry().width()-dlg.width())/2, (QApplication::desktop()->screenGeometry().height()-dlg.height())/2);
        dlg.exec();  
    }        
    else if(item == d->m_pListWidget->item(2+2+2))
    {//系统升级
        OperationTipsFrm dlg(this);
        //系统升级,确定,请索取update.img升级包文件放入U盘(FAT32)，插入U盘即可自动升级!
        dlg.setMessageBox(QObject::tr("SystemUpgrade"), QObject::tr("SystemUpgradeHint"), QObject::tr("Ok"), "", 1);
    }else if(item == d->m_pListWidget->item(3+2+2))
    {//系统维护
        SystemMaintenanceFrm dlg(this);
        dlg.setData(ReadConfig::GetInstance()->getMaintenance_boot(), ReadConfig::GetInstance()->getMaintenance_bootTimer());
        if(dlg.exec() == 0)
        {
            ReadConfig::GetInstance()->setMaintenance_boot(dlg.getTimeMode());
            ReadConfig::GetInstance()->setMaintenance_bootTimer(dlg.getTime());
        }
    }else if(item == d->m_pListWidget->item(4+2+2))
    {//设备关机
        DevicePoweroffFrm dlg(this);
        dlg.exec();
    }
    else if(item == d->m_pListWidget->item(5+2+2))
    {//恢复默认设置
        OperationTipsFrm dlg(this);
        //还原所有设置,还原所有设置后所有配置信息将被清除,是否继续进行该操作？
        int ret = dlg.setMessageBox(QObject::tr("ReturnSetting"), QObject::tr("ReturnSettingHint"));
        if(ret == 0)
        {
#ifdef Q_OS_LINUX
            system("rm -rf /mnt/user/parameters.ini");
            myHelper::Utils_Reboot();
#endif
        }
    }
    else if(item == d->m_pListWidget->item(6+2+2))
    {//恢复出厂设置
        OperationTipsFrm dlg(this);
        //恢复出厂设置,恢复出厂设置后所有数据将被清除,是否继续进行该操作？
        int ret = dlg.setMessageBox(QObject::tr("ReturnToFactorySetting"), QObject::tr("ReturnToFactorySettingHint"));
        if(ret == 0)
        {
#ifdef Q_OS_LINUX
            system("rm -rf /mnt/user/parameters.ini");
            system("rm -rf /mnt/user/facedb/*");
            system("rm -rf /mnt/user/face_crop_image/*");
            myHelper::Utils_Reboot();
#endif
        }
    }
}
#ifdef SCREENCAPTURE  //ScreenCapture 
void AboutMachineFrm::mouseDoubleClickEvent(QMouseEvent* event)
{
    grab().save(QString("/mnt/user/screenshot/%1.png").arg(this->metaObject()->className()),"png");    
}
#endif 