#include "ManagingPeopleFrm.h"

#include "../SetupItemDelegate/CItemWidget.h"
#include "SettingFuncFrms/SysSetupFrms/LanguageFrm.h"

#include <QListWidget>
#include <QHBoxLayout>

class ManagingPeopleFrmPrivate
{
    Q_DECLARE_PUBLIC(ManagingPeopleFrm)
public:
    ManagingPeopleFrmPrivate(ManagingPeopleFrm *dd);
private:
    void InitUI();
    void InitData();
    void InitConnect();
private:
    QListWidget *m_pListWidget;
private:
    ManagingPeopleFrm *const q_ptr;
};

ManagingPeopleFrmPrivate::ManagingPeopleFrmPrivate(ManagingPeopleFrm *dd)
    : q_ptr(dd)
{
    this->InitUI();
    this->InitData();
    this->InitConnect();
}

ManagingPeopleFrm::ManagingPeopleFrm(QWidget *parent)
    : SettingBaseFrm(parent)
    , d_ptr(new ManagingPeopleFrmPrivate(this))
{
}

ManagingPeopleFrm::~ManagingPeopleFrm()
{

}

void ManagingPeopleFrmPrivate::InitUI()
{
    m_pListWidget = new QListWidget;
    QHBoxLayout *layout = new QHBoxLayout(q_func());
    layout->setSpacing(0);
    layout->setMargin(0);
    layout->addWidget(m_pListWidget);
}

void ManagingPeopleFrmPrivate::InitData()
{
    LanguageFrm::GetInstance()->UseLanguage(0); //加上才能翻译     
    QStringList listName;
    //listName<<QObject::tr("新增人员")<<QObject::tr("导入人员")<<QObject::tr("查看人员");
    listName<<QObject::tr("AddPerson")<<QObject::tr("ImportPersonRecord")<<QObject::tr("InqueryPerson");
    for(int i = 0; i<listName.count(); i++)
    {
        QListWidgetItem *pItem = new QListWidgetItem;
        CItemWidget *pItemWidget = new CItemWidget;
        pItemWidget->setData(listName.at(i), ":/Images/SmallRound.png");
        m_pListWidget->addItem(pItem);
        m_pListWidget->setItemWidget(pItem, pItemWidget);
    }
}

void ManagingPeopleFrmPrivate::InitConnect()
{
    QObject::connect(m_pListWidget, &QListWidget::itemClicked, q_func(), &ManagingPeopleFrm::slotIemClicked);
}

void ManagingPeopleFrm::slotIemClicked(QListWidgetItem *item)
{
    Q_D(ManagingPeopleFrm);
#ifdef SCREENCAPTURE  //ScreenCapture       
    grab().save(QString("/mnt/user/screenshot/%1.png").arg(this->metaObject()->className()),"png");        
#endif     
    CItemWidget *pItemWidget = (CItemWidget *)d->m_pListWidget->itemWidget(item);
    emit sigShowFrm(pItemWidget->getNameText());
}
#ifdef SCREENCAPTURE  //ScreenCapture   
void ManagingPeopleFrm::mouseDoubleClickEvent(QMouseEvent* event)
{
    grab().save(QString("/mnt/user/screenshot/%1.png").arg(this->metaObject()->className()),"png");    
} 
#endif 