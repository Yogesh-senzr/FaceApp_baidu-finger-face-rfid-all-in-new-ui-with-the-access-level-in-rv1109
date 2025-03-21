#include "RecordsManagementFrm.h"

#include "../SetupItemDelegate/CItemWidget.h"
#include "ConfigAccessRecordsFrm.h"
#include "Config/ReadConfig.h"

#include "SettingFuncFrms/SysSetupFrms/LanguageFrm.h"

#include <QListWidget>
#include <QHBoxLayout>
#include <QtConcurrent/QtConcurrent>

class RecordsManagementFrmPrivate
{
    Q_DECLARE_PUBLIC(RecordsManagementFrm)
public:
    RecordsManagementFrmPrivate(RecordsManagementFrm *dd);
private:
    void InitUI();
    void InitData();
    void InitConnect();
private:
    void ConfigAccessRecords();
private:
    QListWidget *m_pListWidget;
private:
    RecordsManagementFrm *const q_ptr;
};

RecordsManagementFrmPrivate::RecordsManagementFrmPrivate(RecordsManagementFrm *dd)
    : q_ptr(dd)
{
    this->InitUI();
    this->InitData();
    this->InitConnect();
}

RecordsManagementFrm::RecordsManagementFrm(QWidget *parent)
    : SettingBaseFrm(parent)
    , d_ptr(new RecordsManagementFrmPrivate(this))
{
}

RecordsManagementFrm::~RecordsManagementFrm()
{

}

void RecordsManagementFrmPrivate::InitUI()
{
    m_pListWidget = new QListWidget;
    QHBoxLayout *layout = new QHBoxLayout(q_func());
    layout->setSpacing(0);
    layout->setMargin(0);
    layout->addWidget(m_pListWidget);
}

void RecordsManagementFrmPrivate::InitData()
{
    LanguageFrm::GetInstance()->UseLanguage(0);//从配置中读取    
    QStringList listName;
    listName<<QObject::tr("SettingPassRecord")<<QObject::tr("ViewPassRecord");//设置通行记录,查看通行记录//通行记录上限
    for(int i = 0; i<listName.count(); i++)
    {
        QListWidgetItem *pItem = new QListWidgetItem;
        CItemWidget *pItemWidget = new CItemWidget;;
        pItemWidget->setData(listName.at(i), ":/Images/SmallRound.png");
        m_pListWidget->addItem(pItem);
        m_pListWidget->setItemWidget(pItem, pItemWidget);
    }
}

void RecordsManagementFrmPrivate::InitConnect()
{
    QObject::connect(m_pListWidget, &QListWidget::itemClicked, q_func(), &RecordsManagementFrm::slotIemClicked);
}

void RecordsManagementFrmPrivate::ConfigAccessRecords()
{
    ConfigAccessRecordsFrm dlg(q_func());
    dlg.setInitConfig(ReadConfig::GetInstance()->getRecords_Manager_PanoramaImg(), ReadConfig::GetInstance()->getRecords_Manager_FaceImg(), ReadConfig::GetInstance()->getRecords_Manager_Stranger());
    if(dlg.exec())return;
    //读取用户点确后的参数
    ReadConfig::GetInstance()->setRecords_Manager_PanoramaImg(dlg.getPanoramaState());
    ReadConfig::GetInstance()->setRecords_Manager_FaceImg(dlg.getFaceState());
    ReadConfig::GetInstance()->setRecords_Manager_Stranger(dlg.getStrangerState());
}

void RecordsManagementFrm::setLeaveEvent()
{
    QtConcurrent::run(ReadConfig::GetInstance(), &ReadConfig::setSaveConfig);
}

void RecordsManagementFrm::slotIemClicked(QListWidgetItem *item)
{
    Q_D(RecordsManagementFrm);
    CItemWidget *pItemWidget = (CItemWidget *)d->m_pListWidget->itemWidget(item);
    QString Name = pItemWidget->getNameText();
    if(Name.startsWith(QObject::tr("ViewPassRecord")))emit sigShowFrm(Name);//查看通行记录
    else d->ConfigAccessRecords();
}
#ifdef SCREENCAPTURE  //ScreenCapture   
void RecordsManagementFrm::mouseDoubleClickEvent(QMouseEvent* event)
{
    grab().save(QString("/mnt/user/screenshot/%1.png").arg(this->metaObject()->className()),"png");    
}
#endif 