#include "SrvSetupFrm.h"

#include "../SettingShowTmpFrm.h"
#include <QListWidget>
#include <QHBoxLayout>

class SrvSetupFrmPrivate
{
    Q_DECLARE_PUBLIC(SrvSetupFrm)
public:
    SrvSetupFrmPrivate(SrvSetupFrm *dd);
private:
    void InitUI();
    void InitData();
    void InitConnect();
private:
    QListWidget *m_pListWidget;
private:
    SrvSetupFrm *const q_ptr;
};

SrvSetupFrmPrivate::SrvSetupFrmPrivate(SrvSetupFrm *dd)
    : q_ptr(dd)
{
    this->InitUI();
    this->InitData();
    this->InitConnect();
}


SrvSetupFrm::SrvSetupFrm(QWidget *parent)
    : QWidget(parent)
    , d_ptr(new SrvSetupFrmPrivate(this))
{

}

SrvSetupFrm::~SrvSetupFrm()
{

}

void SrvSetupFrmPrivate::InitUI()
{
    m_pListWidget = new QListWidget;
    QHBoxLayout *layout = new QHBoxLayout(q_func());
    layout->setSpacing(0);
    layout->setMargin(0);
    layout->addWidget(m_pListWidget);
}

void SrvSetupFrmPrivate::InitData()
{
    QStringList listName;
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

void SrvSetupFrmPrivate::InitConnect()
{

}
