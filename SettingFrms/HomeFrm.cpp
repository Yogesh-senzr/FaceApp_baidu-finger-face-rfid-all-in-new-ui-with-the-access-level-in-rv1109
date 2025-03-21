#include "HomeFrm.h"

#include <QToolButton>
#include <QHBoxLayout>
#include <QGridLayout>

class HomeFrmPrivate
{
    Q_DECLARE_PUBLIC(HomeFrm)
public:
    HomeFrmPrivate(HomeFrm *dd);
private:
    void InitUI();
    void InitData();
    void InitConnect();
private:
    QToolButton *m_pManagingPeopleBtn;//人员管理
    QToolButton *m_pRecordsManagementBtn;//记录管理
    QToolButton *m_pNetworkSetupBtn;//网络设置
    QToolButton *m_pSrvSetupBtn;//服务器设置
    QToolButton *m_pSysSetupBtn;//系统设置
    QToolButton *m_pIdentifySetupBtn;//识别设置
private:
    HomeFrm *const q_ptr;
};


HomeFrmPrivate::HomeFrmPrivate(HomeFrm *dd)
    : q_ptr(dd)
{
    this->InitUI();
    this->InitData();
    this->InitConnect();
}

HomeFrm::HomeFrm(QWidget *parent)
    : QWidget(parent)
    , d_ptr(new HomeFrmPrivate(this))
{

}

HomeFrm::~HomeFrm()
{

}

void HomeFrmPrivate::InitUI()
{
    m_pManagingPeopleBtn = new QToolButton;//人员管理
    m_pRecordsManagementBtn = new QToolButton;//记录管理
    m_pNetworkSetupBtn = new QToolButton;//网络设置
    m_pSrvSetupBtn = new QToolButton;//服务器设置
    m_pSysSetupBtn = new QToolButton;//系统设置
    m_pIdentifySetupBtn = new QToolButton;//识别设置

    QGridLayout *gridLayout = new QGridLayout();
    gridLayout->addWidget(m_pManagingPeopleBtn, 0, 0);
    gridLayout->addWidget(m_pRecordsManagementBtn, 0, 1);
    gridLayout->addWidget(m_pNetworkSetupBtn, 1, 0);
    gridLayout->addWidget(m_pSrvSetupBtn, 1, 1);
    gridLayout->addWidget(m_pSysSetupBtn, 2, 0);
    gridLayout->addWidget(m_pIdentifySetupBtn, 2, 1);
    // 设置水平间距
    gridLayout->setHorizontalSpacing(30);
    // 设置垂直间距
    gridLayout->setVerticalSpacing(30);

    QVBoxLayout *vLayout = new QVBoxLayout();
    vLayout->addLayout(gridLayout);
    vLayout->addSpacing(80);

    QHBoxLayout *hLayout = new QHBoxLayout(q_func());
    hLayout->setMargin(0);
    hLayout->addSpacing(40);
    hLayout->addLayout(vLayout);
    hLayout->addSpacing(40);
}

void HomeFrmPrivate::InitData()
{
    m_pManagingPeopleBtn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    m_pManagingPeopleBtn->setText("人员管理");
    m_pManagingPeopleBtn->setIcon(QIcon(":/Images/ManagingPeople.png"));
    m_pManagingPeopleBtn->setIconSize(QSize(64, 64));

    m_pRecordsManagementBtn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    m_pRecordsManagementBtn->setText("记录管理");
    m_pRecordsManagementBtn->setIcon(QIcon(":/Images/RecordsManagement.png"));
    m_pRecordsManagementBtn->setIconSize(QSize(64, 64));

    m_pNetworkSetupBtn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    m_pNetworkSetupBtn->setText("网络设置");
    m_pNetworkSetupBtn->setIcon(QIcon(":/Images/NetworkSetup.png"));
    m_pNetworkSetupBtn->setIconSize(QSize(64, 64));

    m_pSrvSetupBtn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    m_pSrvSetupBtn->setText("服务器设置");
    m_pSrvSetupBtn->setIcon(QIcon(":/Images/SrvSetup.png"));
    m_pSrvSetupBtn->setIconSize(QSize(64, 64));

    m_pSysSetupBtn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    m_pSysSetupBtn->setText("系统设置");
    m_pSysSetupBtn->setIcon(QIcon(":/Images/SysSetup.png"));
    m_pSysSetupBtn->setIconSize(QSize(64, 64));

    m_pIdentifySetupBtn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    m_pIdentifySetupBtn->setText("识别设置");
    m_pIdentifySetupBtn->setIcon(QIcon(":/Images/IdentifySetup.png"));
    m_pIdentifySetupBtn->setIconSize(QSize(64, 64));
}

void HomeFrmPrivate::InitConnect()
{

}

#ifdef SCREENCAPTURE  //ScreenCapture 
void HomeFrm::mouseDoubleClickEvent(QMouseEvent* event)
{
    grab().save(QString("/mnt/user/screenshot/%1.png").arg(this->metaObject()->className()),"png");    
}	
#endif 