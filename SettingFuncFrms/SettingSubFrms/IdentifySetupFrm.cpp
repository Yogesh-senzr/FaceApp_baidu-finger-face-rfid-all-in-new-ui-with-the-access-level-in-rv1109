#include "IdentifySetupFrm.h"

#include "../SettingShowTmpFrm.h"
#include <QListWidget>
#include <QHBoxLayout>

class IdentifySetupFrmPrivate
{
    Q_DECLARE_PUBLIC(IdentifySetupFrm)
public:
    IdentifySetupFrmPrivate(IdentifySetupFrm *dd);
private:
    void InitUI();
    void InitData();
    void InitConnect();
private:
    QListWidget *m_pListWidget;
private:
    IdentifySetupFrm *const q_ptr;
};

IdentifySetupFrmPrivate::IdentifySetupFrmPrivate(IdentifySetupFrm *dd)
    : q_ptr(dd)
{
    this->InitUI();
    this->InitData();
    this->InitConnect();
}

IdentifySetupFrm::IdentifySetupFrm(QWidget *parent)
    : QWidget(parent)
    , d_ptr(new IdentifySetupFrmPrivate(this))
{

}

IdentifySetupFrm::~IdentifySetupFrm()
{

}

void IdentifySetupFrmPrivate::InitUI()
{
    m_pListWidget = new QListWidget;
    QHBoxLayout *layout = new QHBoxLayout(q_func());
    layout->setSpacing(0);
    layout->setMargin(0);
    layout->addWidget(m_pListWidget);
}

void IdentifySetupFrmPrivate::InitData()
{
    m_pListWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QStringList listName;
    listName<<"人脸识别"<<"体温检测"<<"温度补偿"<<"报警温度"<<"红外图像"<<"识别阀值"<<"活体检测"<<"活体阀值"<<"口罩检测"<<"识别间隔"<<"识别距离"<<"语音模式"<<"问候语"<<"陌生人语音"<<"曝光补偿";

    for(int i = 0; i<listName.count(); i++)
    {
        QListWidgetItem *pItem = new QListWidgetItem;
        SettingShowTmpFrm *pItemWidget = new SettingShowTmpFrm;
        pItem->setSizeHint(QSize(800, 80));
        pItemWidget->setData(listName.at(i), ":/Images/SmallRound.png");
        m_pListWidget->addItem(pItem);
        m_pListWidget->setItemWidget(pItem, pItemWidget);
    }
}

void IdentifySetupFrmPrivate::InitConnect()
{

}
