#include "RecordsManagementFrm.h"

#include "../SettingShowTmpFrm.h"
#include <QListWidget>
#include <QHBoxLayout>

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
    : QWidget(parent)
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
    QStringList listName;
    listName<<"设置通行记录"<<"查看通行记录";
    for(int i = 0; i<listName.count(); i++)
    {
        QListWidgetItem *pItem = new QListWidgetItem;
        SettingShowTmpFrm *pItemWidget = new SettingShowTmpFrm;
        pItem->setSizeHint(QSize(800, 90));
        pItemWidget->setData(listName.at(i), ":/Images/SmallRound.png");
        m_pListWidget->addItem(pItem);
        m_pListWidget->setItemWidget(pItem, pItemWidget);;
    }
}

void RecordsManagementFrmPrivate::InitConnect()
{

}
