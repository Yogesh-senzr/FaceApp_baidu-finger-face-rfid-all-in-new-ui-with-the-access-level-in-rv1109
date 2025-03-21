#include "SrvSetupFrm.h"

#include "Config/ReadConfig.h"
#include "MessageHandler/Log.h"
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QHBoxLayout>
#include <QSettings>
#include <QTextCodec>

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
    QLineEdit *m_pHttpServerAddress;
    QLineEdit *m_pHttpServerPassword;

    QLineEdit *m_pPostPersonRecordHttpServerAddress;
    QLineEdit *m_pPostPersonRecordHttpServerPassword;

    QLineEdit *m_pNewRegisteredPersonHttpServerAddress;
    QLineEdit *m_pNewRegisteredPersonHttpServerPassword;

    QLineEdit *m_pSyncUsersHttpServerAddress;
    QLineEdit *m_pSyncUsersHttpServerPassword;

    QLineEdit *m_pUserDetailHttpServerAddress;
    QLineEdit *m_pUserDetailHttpServerPassword;    


    QPushButton *m_pConfirmButton;
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
    : SettingBaseFrm(parent)
    , d_ptr(new SrvSetupFrmPrivate(this))
{

}

SrvSetupFrm::~SrvSetupFrm()
{

}

void SrvSetupFrmPrivate::InitUI()
{
	m_pHttpServerAddress = new QLineEdit;
	m_pHttpServerPassword = new QLineEdit;
    m_pPostPersonRecordHttpServerAddress = new QLineEdit;
    m_pPostPersonRecordHttpServerPassword = new QLineEdit;
    m_pNewRegisteredPersonHttpServerAddress = new QLineEdit;
    m_pNewRegisteredPersonHttpServerPassword = new QLineEdit;
    m_pSyncUsersHttpServerAddress = new QLineEdit;
    m_pSyncUsersHttpServerPassword = new QLineEdit;
    m_pUserDetailHttpServerAddress = new QLineEdit;
    m_pUserDetailHttpServerPassword = new QLineEdit; 

    m_pHttpServerAddress->setContextMenuPolicy(Qt::NoContextMenu);
    m_pHttpServerPassword->setContextMenuPolicy(Qt::NoContextMenu);
    m_pPostPersonRecordHttpServerAddress->setContextMenuPolicy(Qt::NoContextMenu);
    m_pPostPersonRecordHttpServerPassword->setContextMenuPolicy(Qt::NoContextMenu);
    m_pNewRegisteredPersonHttpServerAddress->setContextMenuPolicy(Qt::NoContextMenu);  
    m_pNewRegisteredPersonHttpServerPassword->setContextMenuPolicy(Qt::NoContextMenu);
    m_pSyncUsersHttpServerAddress->setContextMenuPolicy(Qt::NoContextMenu);  
    m_pSyncUsersHttpServerPassword->setContextMenuPolicy(Qt::NoContextMenu);
    m_pUserDetailHttpServerAddress->setContextMenuPolicy(Qt::NoContextMenu);  
    m_pUserDetailHttpServerPassword->setContextMenuPolicy(Qt::NoContextMenu);

    m_pConfirmButton = new QPushButton;//确定
    QVBoxLayout *vLayout = new QVBoxLayout;
    vLayout->addWidget(new QLabel(QObject::tr("HTTPServerAddress")));//HTTP服务器地址
    vLayout->addWidget(m_pHttpServerAddress);

    QVBoxLayout *vLayout1 = new QVBoxLayout;
    vLayout1->addWidget(new QLabel(QObject::tr("HTTPServerPassword")));//HTTP服务器密码
    vLayout1->addWidget(m_pHttpServerPassword);

    QVBoxLayout *vLayout2 = new QVBoxLayout;
    vLayout2->addWidget(new QLabel(QObject::tr("PostPersonRecordHttpServerAddress")));//HTTP离线推送识别记录服务器地址
    vLayout2->addWidget(m_pPostPersonRecordHttpServerAddress);

    QVBoxLayout *vLayout3 = new QVBoxLayout;
    vLayout3->addWidget(new QLabel(QObject::tr("PostPersonRecordHttpServerPassword")));//HTTP离线推送识别记录服务器密码
    vLayout3->addWidget(m_pPostPersonRecordHttpServerPassword);

    QVBoxLayout *vLayout4 = new QVBoxLayout;
    vLayout4->addWidget(new QLabel(QObject::tr("NewRegisteredPersonHttpServerAddress")));
    vLayout4->addWidget(m_pNewRegisteredPersonHttpServerAddress);

    QVBoxLayout *vLayout5 = new QVBoxLayout;
    vLayout5->addWidget(new QLabel(QObject::tr("NewRegisteredPersonHttpServerPassword")));
    vLayout5->addWidget(m_pNewRegisteredPersonHttpServerPassword);

    QVBoxLayout *vLayout6 = new QVBoxLayout;
    vLayout6->addWidget(new QLabel(QObject::tr("SyncUsersHttpServerAddress")));
    vLayout6->addWidget(m_pSyncUsersHttpServerAddress);

    QVBoxLayout *vLayout7 = new QVBoxLayout;
    vLayout7->addWidget(new QLabel(QObject::tr("SyncUsersHttpServerPassword")));
    vLayout7->addWidget(m_pSyncUsersHttpServerPassword);

    QVBoxLayout *vLayout8 = new QVBoxLayout;
    vLayout8->addWidget(new QLabel(QObject::tr("UserDetailHttpServerAddress")));
    vLayout8->addWidget(m_pUserDetailHttpServerAddress);

    QVBoxLayout *vLayout9 = new QVBoxLayout;
    vLayout9->addWidget(new QLabel(QObject::tr("UserDetailHttpServerPassword")));
    vLayout9->addWidget(m_pUserDetailHttpServerPassword);


    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->addStretch();
    hLayout->addWidget(m_pConfirmButton);
    hLayout->addStretch();

    QVBoxLayout *malayout = new QVBoxLayout(q_func());
    malayout->setSpacing(0);
    malayout->setContentsMargins(30, 0, 30, 20);
    malayout->addLayout(vLayout);
    malayout->addSpacing(5);
    malayout->addLayout(vLayout1);
    malayout->addSpacing(5);
    malayout->addLayout(vLayout2);
    malayout->addSpacing(5);
    malayout->addLayout(vLayout3);
    malayout->addSpacing(5);
    malayout->addLayout(vLayout4);
    malayout->addSpacing(5);
    malayout->addLayout(vLayout5);
    malayout->addSpacing(5);
    malayout->addLayout(vLayout6);
    malayout->addSpacing(5);
    malayout->addLayout(vLayout7);
    malayout->addSpacing(5);
    malayout->addLayout(vLayout8);
    malayout->addSpacing(5);
    malayout->addLayout(vLayout9);
    malayout->addSpacing(10);
    malayout->addLayout(hLayout);

    malayout->addStretch();
}

void SrvSetupFrmPrivate::InitData()
{
    m_pConfirmButton->setFixedSize(282, 62);
    m_pConfirmButton->setText(QObject::tr("Save"));//保存 
}

void SrvSetupFrmPrivate::InitConnect()
{
    QObject::connect(m_pConfirmButton, &QPushButton::clicked, q_func(), &SrvSetupFrm::slotConfirmButton);
}

void SrvSetupFrm::setEnter()
{
    Q_D(SrvSetupFrm);

    d->m_pHttpServerAddress->setText(ReadConfig::GetInstance()->getSrv_Manager_Address());
    d->m_pHttpServerPassword->setText(ReadConfig::GetInstance()->getSrv_Manager_Password());
    d->m_pPostPersonRecordHttpServerAddress->setText(ReadConfig::GetInstance()->getPost_PersonRecord_Address());
    d->m_pPostPersonRecordHttpServerPassword->setText(ReadConfig::GetInstance()->getPost_PersonRecord_Password());
    d->m_pNewRegisteredPersonHttpServerAddress->setText(ReadConfig::GetInstance()->getPerson_Registration_Address());
    d->m_pNewRegisteredPersonHttpServerPassword->setText(ReadConfig::GetInstance()->getPerson_Registration_Password());
    d->m_pSyncUsersHttpServerAddress->setText(ReadConfig::GetInstance()->getSyncUsersAddress());
    d->m_pSyncUsersHttpServerPassword->setText(ReadConfig::GetInstance()->getSyncUsersPassword());
    d->m_pUserDetailHttpServerAddress->setText(ReadConfig::GetInstance()->getUserDetailAddress());
    d->m_pUserDetailHttpServerPassword->setText(ReadConfig::GetInstance()->getUserDetailPassword());

    // Add this debug log
    LogD("Loading Person Registration URL: %s\n", 
         ReadConfig::GetInstance()->getPerson_Registration_Address().toStdString().c_str());

}


void SrvSetupFrm::slotConfirmButton()
{
	Q_D(SrvSetupFrm);
	ReadConfig::GetInstance()->setSrv_Manager_Address(d->m_pHttpServerAddress->text());
	ReadConfig::GetInstance()->setSrv_Manager_Password(d->m_pHttpServerPassword->text());
	ReadConfig::GetInstance()->setPost_PersonRecord_Address(d->m_pPostPersonRecordHttpServerAddress->text());
	ReadConfig::GetInstance()->setPost_PersonRecord_Password(d->m_pPostPersonRecordHttpServerPassword->text());
    ReadConfig::GetInstance()->setPerson_Registration_Address(d->m_pNewRegisteredPersonHttpServerAddress->text());
    ReadConfig::GetInstance()->setPerson_Registration_Password(d->m_pNewRegisteredPersonHttpServerPassword->text());
    ReadConfig::GetInstance()->setSyncUsersAddress(d->m_pSyncUsersHttpServerAddress->text());
    ReadConfig::GetInstance()->setSyncUsersPassword(d->m_pSyncUsersHttpServerPassword->text());
    ReadConfig::GetInstance()->setUserDetailAddress(d->m_pUserDetailHttpServerAddress->text());
    ReadConfig::GetInstance()->setUserDetailPassword(d->m_pUserDetailHttpServerPassword->text());
	ReadConfig::GetInstance()->setSaveConfig();


}

void SrvSetupFrm::slotIemClicked(QListWidgetItem */*item*/)
{

}
#ifdef SCREENCAPTURE  //ScreenCapture  
void SrvSetupFrm::mouseDoubleClickEvent(QMouseEvent* event)
{
    grab().save(QString("/mnt/user/screenshot/%1.png").arg(this->metaObject()->className()),"png");    
}
#endif 
