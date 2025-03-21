#include "SysSetupFrm.h"

#include "../SettingShowTmpFrm.h"
#include <QListWidget>
#include <QHBoxLayout>

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
    QListWidget *m_pListWidget;
private:
    SysSetupFrm *const q_ptr;
};

SysSetupFrmPrivate::SysSetupFrmPrivate(SysSetupFrm *dd)
    : q_ptr(dd)
{
    this->InitUI();
    this->InitData();
    this->InitConnect();
}


SysSetupFrm::SysSetupFrm(QWidget *parent)
    : QWidget(parent)
    , d_ptr(new SysSetupFrmPrivate(this))
{

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
    QStringList listName;
    //listName<<"语言设置"<<"亮度设置"<<"补光设置"<<"音量设置"<<"门禁设置"<<"主界面设置"<<"时间设置"<<"RTSP设置"<<"存储容量"<<"关于本机";
    listName<<QObject::tr("LanguageSettings")<<QObject::tr("BrightnessSetting")<<QObject::tr("FillLightSetting")
        <<QObject::tr("VolumeSetting")<<QObject::tr("AccessControlSettings")<<QObject::tr("MainUISettings")
        <<QObject::tr("TimeSetting")<<QObject::tr("RTSPSetting")<<QObject::tr("StorageCapacity ")
        <<QObject::tr("About");

    for(int i = 0; i<listName.count(); i++)
    {
        QListWidgetItem *pItem = new QListWidgetItem;
        SettingShowTmpFrm *pItemWidget = new SettingShowTmpFrm;
        pItem->setSizeHint(QSize(800, 90));
        pItemWidget->setData(listName.at(i), ":/Images/SmallRound.png");
        m_pListWidget->addItem(pItem);
        m_pListWidget->setItemWidget(pItem, pItemWidget);
    }
}

void SysSetupFrmPrivate::InitConnect()
{

}
