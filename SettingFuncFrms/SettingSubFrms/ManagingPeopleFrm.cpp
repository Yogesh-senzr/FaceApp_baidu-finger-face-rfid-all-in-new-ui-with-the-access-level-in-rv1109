#include "ManagingPeopleFrm.h"

#include "../SettingShowTmpFrm.h"
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
    : QWidget(parent)
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
    QStringList listName;
    listName<<"新增人员"<<"批量导入人员"<<"查看人员";
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

void ManagingPeopleFrmPrivate::InitConnect()
{

}
