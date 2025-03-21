#include "NetworkSetupFrm.h"

#include "../SettingShowTmpFrm.h"
#include <QListWidget>
#include <QHBoxLayout>
#include <fcntl.h>
#include <unistd.h>

class NetworkSetupFrmPrivate
{
    Q_DECLARE_PUBLIC(NetworkSetupFrm)
public:
    NetworkSetupFrmPrivate(NetworkSetupFrm *dd);
private:
    void InitUI();
    void InitData();
    void InitConnect();
private:
    QListWidget *m_pListWidget;
private:
    NetworkSetupFrm *const q_ptr;
};

NetworkSetupFrmPrivate::NetworkSetupFrmPrivate(NetworkSetupFrm *dd)
    : q_ptr(dd)
{
    this->InitUI();
    this->InitData();
    this->InitConnect();
}

NetworkSetupFrm::NetworkSetupFrm(QWidget *parent)
    : QWidget(parent)
    , d_ptr(new NetworkSetupFrmPrivate(this))
{

}

NetworkSetupFrm::~NetworkSetupFrm()
{

}

void NetworkSetupFrmPrivate::InitUI()
{
    m_pListWidget = new QListWidget;
    QHBoxLayout *layout = new QHBoxLayout(q_func());
    layout->setSpacing(0);
    layout->setMargin(0);
    layout->addWidget(m_pListWidget);
}

void NetworkSetupFrmPrivate::InitData()
{
    QStringList listName;
    if(!access("/sys/class/net/wlan0/",F_OK))
    {
    	listName<<"Wi-FiSetup"; //Wi-Fi设置
    }
    listName<<"EthernetSetup"; //网口设置
    if(!access("/sys/class/net/p2p0/",F_OK))
    {
    	listName<<QObject::tr("4G"); //4G设置
    }
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

void NetworkSetupFrmPrivate::InitConnect()
{

}
