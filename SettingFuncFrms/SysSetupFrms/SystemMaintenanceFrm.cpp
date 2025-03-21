#include "SystemMaintenanceFrm.h"
#include "Config/ReadConfig.h"
#include <QtWidgets/QLabel>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QTimeEdit>
#include <QtCore/QTime>

class SystemMaintenanceFrmPrivate
{
    Q_DECLARE_PUBLIC(SystemMaintenanceFrm)
public:
    SystemMaintenanceFrmPrivate(SystemMaintenanceFrm *dd);
private:
    void InitUI();
    void InitData();
    void InitConnect();
private:
    QButtonGroup *m_pAutoRebootGroup;
    QTimeEdit *m_pTimeEdit;
    QPushButton *m_pConfirmButton;//确定
    QPushButton *m_pCancelButton;//取消
private:
    SystemMaintenanceFrm *const q_ptr;
};

SystemMaintenanceFrmPrivate::SystemMaintenanceFrmPrivate(SystemMaintenanceFrm *dd)
    : q_ptr(dd)
{
    this->InitUI();
    this->InitData();
    this->InitConnect();
}

SystemMaintenanceFrm::SystemMaintenanceFrm(QWidget *parent)
    : QDialog(parent, Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint)
    , d_ptr(new SystemMaintenanceFrmPrivate(this))
{

}

SystemMaintenanceFrm::~SystemMaintenanceFrm()
{

}

void SystemMaintenanceFrmPrivate::InitUI()
{
    m_pAutoRebootGroup = new QButtonGroup;
    m_pAutoRebootGroup->addButton(new QRadioButton(QObject::tr("None")), 0);//无
    m_pAutoRebootGroup->addButton(new QRadioButton(QObject::tr("Day")), 1);//日

    m_pConfirmButton = new QPushButton;//确定
    m_pCancelButton = new QPushButton;//取消
    m_pTimeEdit = new QTimeEdit;
    m_pTimeEdit->setContextMenuPolicy(Qt::NoContextMenu);


    QHBoxLayout *layout = new QHBoxLayout;
    layout->setMargin(0);
    layout->addStretch();
    layout->addWidget(new QLabel(QObject::tr("SystemMaintenance")));//系统维护
    layout->addStretch();

    QHBoxLayout *hlayout = new QHBoxLayout;
    hlayout->addSpacing(10);
    hlayout->addWidget(new QLabel(QObject::tr("AutomaticReboot")));//自动重启
    hlayout->addStretch();
    hlayout->addWidget(m_pAutoRebootGroup->button(0));
    hlayout->addWidget(m_pAutoRebootGroup->button(1));
    hlayout->addSpacing(10);

    QHBoxLayout *hlayout1 = new QHBoxLayout;
    hlayout1->addSpacing(10);
    hlayout1->addWidget(new QLabel(QObject::tr("RebootTime")));//重启时间
    hlayout1->addStretch();
    hlayout1->addWidget(m_pTimeEdit);
    hlayout1->addSpacing(10);

    QHBoxLayout *hlayout2 = new QHBoxLayout;
    hlayout2->addSpacing(10);
    hlayout2->addWidget(m_pConfirmButton);
    hlayout2->addWidget(m_pCancelButton);
    hlayout2->addSpacing(10);

    QVBoxLayout *vLayout = new QVBoxLayout(q_func());
    vLayout->setContentsMargins(0, 5, 0, 10);
    vLayout->addLayout(layout);
    vLayout->addLayout(hlayout);
    vLayout->addLayout(hlayout1);
    vLayout->addSpacing(10);
    vLayout->addLayout(hlayout2);
}

void SystemMaintenanceFrmPrivate::InitData()
{
    q_func()->setObjectName("SystemMaintenanceFrm");

    m_pAutoRebootGroup->button(0)->setChecked(true);

    m_pConfirmButton->setFixedHeight(64);
    m_pCancelButton->setFixedHeight(64);

    m_pConfirmButton->setText(QObject::tr("Ok"));//确定
    m_pCancelButton->setText(QObject::tr("Cancel"));//取消

    m_pTimeEdit->setDisplayFormat("hh:mm:ss");
    m_pTimeEdit->setTime(QTime::currentTime());
    m_pTimeEdit->setFixedSize(198, 52);
}

void SystemMaintenanceFrmPrivate::InitConnect()
{
    QObject::connect(m_pConfirmButton, &QPushButton::clicked, [&] {

			ReadConfig::GetInstance()->setMaintenance_boot(m_pAutoRebootGroup->checkedId());
            printf("%s %s[%d]  \n", __FILE__, __FUNCTION__, __LINE__); 
			ReadConfig::GetInstance()->setMaintenance_bootTimer(m_pTimeEdit->text());
			
			ReadConfig::GetInstance()->setSaveConfig();
			
			q_func()->done(0); 
		});
    QObject::connect(m_pCancelButton, &QPushButton::clicked, [&] {q_func()->done(1); });
}

void SystemMaintenanceFrm::setData(const int &mode, const QString &time)
{
    Q_D(SystemMaintenanceFrm);
    int index = mode > 1 ? 1: mode;
    d->m_pAutoRebootGroup->button(index)->setChecked(true);
    d->m_pTimeEdit->setTime(QTime::fromString(time));
	ReadConfig::GetInstance()->setMaintenance_boot(mode);
    printf("%s %s[%d]  \n", __FILE__, __FUNCTION__, __LINE__); 
	ReadConfig::GetInstance()->setMaintenance_bootTimer(time);	
}

int SystemMaintenanceFrm::getTimeMode() const
{
    return d_func()->m_pAutoRebootGroup->checkedId();
}

QString SystemMaintenanceFrm::getTime() const
{
    return d_func()->m_pTimeEdit->text();
}

void SystemMaintenanceFrm::slotConfirmButton()
{
	Q_D(SystemMaintenanceFrm);
	int boot = 1;
	for(int i = 0 ;i < 2; i++)
	{
		if(d->m_pAutoRebootGroup->button(i)->isChecked())
		{
			boot = i;
			break;
		}
	}

	ReadConfig::GetInstance()->setMaintenance_boot(boot);
    printf("%s %s[%d]  \n", __FILE__, __FUNCTION__, __LINE__); 
	ReadConfig::GetInstance()->setMaintenance_bootTimer(d->m_pTimeEdit->text());
	ReadConfig::GetInstance()->setSaveConfig();
}
#ifdef SCREENCAPTURE  //ScreenCapture 
void SystemMaintenanceFrm::mouseDoubleClickEvent(QMouseEvent* event)
{
    grab().save(QString("/mnt/user/screenshot/%1.png").arg(this->metaObject()->className()),"png");    
}
#endif 