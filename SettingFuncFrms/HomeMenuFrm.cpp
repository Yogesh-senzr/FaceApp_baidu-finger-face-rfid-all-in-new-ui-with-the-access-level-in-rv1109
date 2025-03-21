#include "HomeMenuFrm.h"
#include "SettingFuncFrms/SysSetupFrms/LanguageFrm.h"

#include <QToolButton>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QStyleOption>
#include <QPainter>
#include <QMessageBox>

class HomeMenuFrmPrivate
{
    Q_DECLARE_PUBLIC(HomeMenuFrm)
public:
    HomeMenuFrmPrivate(HomeMenuFrm *dd);
private:
    void InitUI();
    void InitData();
    void InitConnect();
private:
    QToolButton *m_pManagingPeopleBtn;//人员管理
    QToolButton *m_pRecordsManagementBtn;//记录管理
    QToolButton *m_pNetworkSetupBtn;//网络设置
    QToolButton *m_pSrvSetupBtn;//服务器设置
    QToolButton *m_pSysSetupBtn;//系统配置
    QToolButton *m_pIdentifySetupBtn;//识别设置  
private:
    HomeMenuFrm *const q_ptr;
};

HomeMenuFrmPrivate::HomeMenuFrmPrivate(HomeMenuFrm *dd)
    : q_ptr(dd)
{
    this->InitUI();
    this->InitData();
    this->InitConnect();
}

HomeMenuFrm::HomeMenuFrm(QWidget *parent)
    : SettingBaseFrm(parent)
    , d_ptr(new HomeMenuFrmPrivate(this))
{
//    QPalette pal;
//    pal.setColor(QPalette::Background, QColor(2,27,83,255));
//    this->setAutoFillBackground(true);
//    this->setPalette(pal);
 
}

#ifdef SCREENCAPTURE  //ScreenCapture     
void HomeMenuFrm::mouseDoubleClickEvent(QMouseEvent* event)
{
    grab().save(QString("/mnt/user/screenshot/%1.png").arg(this->metaObject()->className()),"png");   
}	
#endif 
HomeMenuFrm::~HomeMenuFrm()
{

}

void HomeMenuFrmPrivate::InitUI()
{
    m_pManagingPeopleBtn = new QToolButton;//人员管理
    m_pRecordsManagementBtn = new QToolButton;//记录管理
    m_pNetworkSetupBtn = new QToolButton;//网络设置
    m_pSrvSetupBtn = new QToolButton;//服务器设置
    m_pSysSetupBtn = new QToolButton;//系统配置
    m_pIdentifySetupBtn = new QToolButton;//识别设置

    QGridLayout *gridLayout = new QGridLayout;
    gridLayout->addWidget(m_pManagingPeopleBtn, 0, 0);
    gridLayout->addWidget(m_pRecordsManagementBtn, 0, 1);
    gridLayout->addWidget(m_pNetworkSetupBtn, 1, 0);
    gridLayout->addWidget(m_pSrvSetupBtn, 1, 1);
    gridLayout->addWidget(m_pIdentifySetupBtn, 2, 0);
        gridLayout->addWidget(m_pSysSetupBtn, 2, 1);
    gridLayout->setHorizontalSpacing(30);
    gridLayout->setVerticalSpacing(30);

    QVBoxLayout *vLayout = new QVBoxLayout;
    vLayout->addLayout(gridLayout);
    vLayout->addSpacing(80);

    QHBoxLayout *hLayout = new QHBoxLayout(q_func());
    hLayout->setMargin(0);
    hLayout->addSpacing(40);
    hLayout->addLayout(vLayout);
    hLayout->addSpacing(40);
}

void HomeMenuFrmPrivate::InitData()
{
    m_pManagingPeopleBtn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    m_pManagingPeopleBtn->setText(QObject::tr("PersonManaging"));//人员管理
        
    m_pManagingPeopleBtn->setIcon(QIcon(":/Images/ManagingPeople.png"));
    m_pManagingPeopleBtn->setIconSize(QSize(64, 64));
     
    m_pRecordsManagementBtn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    m_pRecordsManagementBtn->setText(QObject::tr("RecordsManagement"));//记录管理
    m_pRecordsManagementBtn->setIcon(QIcon(":/Images/RecordsManagement.png"));
    m_pRecordsManagementBtn->setIconSize(QSize(64, 64));

    m_pNetworkSetupBtn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    m_pNetworkSetupBtn->setText(QObject::tr("NetworkSetup"));//网络设置
    m_pNetworkSetupBtn->setIcon(QIcon(":/Images/NetworkSetup.png"));
    m_pNetworkSetupBtn->setIconSize(QSize(64, 64));

    m_pSrvSetupBtn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    m_pSrvSetupBtn->setText(QObject::tr("HostSetup"));//服务器设置
    m_pSrvSetupBtn->setIcon(QIcon(":/Images/SrvSetup.png"));
    m_pSrvSetupBtn->setIconSize(QSize(64, 64));

    m_pSysSetupBtn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    m_pSysSetupBtn->setText(QObject::tr("SystemSetup"));//系统配置
    m_pSysSetupBtn->setIcon(QIcon(":/Images/SysSetup.png"));
    m_pSysSetupBtn->setIconSize(QSize(64, 64));

    m_pIdentifySetupBtn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    m_pIdentifySetupBtn->setText(QObject::tr("IdentifySetup"));//识别设置
    m_pIdentifySetupBtn->setIcon(QIcon(":/Images/IdentifySetup.png"));
    m_pIdentifySetupBtn->setIconSize(QSize(64, 64));
    
}

void HomeMenuFrmPrivate::InitConnect()
{
    QObject::connect(m_pManagingPeopleBtn, &QToolButton::clicked, q_func(), &HomeMenuFrm::slotManagingPeopleClicked);//人员管理
    QObject::connect(m_pRecordsManagementBtn, &QToolButton::clicked, q_func(), &HomeMenuFrm::slotRecordsManagementClicked);//记录管理
    QObject::connect(m_pNetworkSetupBtn, &QToolButton::clicked, q_func(), &HomeMenuFrm::slotNetworkSetupClicked);//网络设置
    QObject::connect(m_pSrvSetupBtn, &QToolButton::clicked, q_func(), &HomeMenuFrm::slotSrvSetupClicked);//服务器设置
    QObject::connect(m_pSysSetupBtn, &QToolButton::clicked, q_func(), &HomeMenuFrm::slotSysSetupClicked);//系统配置
    QObject::connect(m_pIdentifySetupBtn, &QToolButton::clicked, q_func(), &HomeMenuFrm::slotIdentifySetupClicked);//识别设置

}


void HomeMenuFrm::slotManagingPeopleClicked()
{
    emit sigShowFrm(QObject::tr("PersonManaging"));//人员管理
}

void HomeMenuFrm::slotRecordsManagementClicked()
{
    emit sigShowFrm(QObject::tr("RecordsManagement"));//记录管理
}

void HomeMenuFrm::slotNetworkSetupClicked()
{
    emit sigShowFrm(QObject::tr("NetworkSetup"));//网络设置
}

void HomeMenuFrm::slotSrvSetupClicked()
{
    QMessageBox warningMsg;
    warningMsg.setIcon(QMessageBox::Warning);
    warningMsg.setText(tr("Warning"));
    warningMsg.setInformativeText(tr("Entering HostSetup may affect Server configuration. Proceed with caution."));
    warningMsg.setStandardButtons(QMessageBox::Cancel | QMessageBox::Ok);
    warningMsg.setDefaultButton(QMessageBox::Cancel);

    // Set font size
    QFont font = warningMsg.font();
    font.setPointSize(8);
    warningMsg.setFont(font);

    // Set style sheet for checkbox and buttons
    warningMsg.setStyleSheet(
        "QMessageBox {"
        "    background-color: #FEFEFA;"
        "    color: #000000;"
        "}"
        "QMessageBox QPushButton {"
        "    background-color: #0087BD;" // Dark gray button background
        "    color: #FEFEFA;"
        "    min-width: 54px;"
        "    padding: 5px;"
        "    border-radius: 0px;"
        "}"
        "QMessageBox QPushButton:hover {"
        "    background-color: #33A3D6;" // Slightly lighter when hovering
        "}"
    );

    if (warningMsg.exec() == QMessageBox::Ok) {
        emit sigShowFrm(tr("HostSetup"));
    }
}
void HomeMenuFrm::slotSysSetupClicked()
{
    emit sigShowFrm(QObject::tr("SystemSetup"));//系统配置
}

void HomeMenuFrm::slotIdentifySetupClicked()
{
    emit sigShowFrm(QObject::tr("IdentifySetup"));//识别设置
}
