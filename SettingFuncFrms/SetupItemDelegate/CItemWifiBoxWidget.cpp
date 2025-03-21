#include "CItemWifiBoxWidget.h"
#include "Config/ReadConfig.h"
#include "MessageHandler/Log.h"


#include <unistd.h>
#include <QLabel>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QPushButton>
#include <QMovie>
#include <QtConcurrent/QtConcurrent>
#include <QApplication>
#include <QDesktopWidget>

class CItemWifiBoxWidgetPrivate
{
    Q_DECLARE_PUBLIC(CItemWifiBoxWidget)
public:
    CItemWifiBoxWidgetPrivate(CItemWifiBoxWidget *dd);
private:
    void InitUI();
    void InitData();
    void InitConnect();
private:
    QLabel *m_pNameLabel;
    QPushButton *m_pRebootButton;
    QLabel *m_pIconLabel;
    QCheckBox *m_pWifiSwitchBox;
private:
    CItemWifiBoxWidget *const q_ptr;
};

CItemWifiBoxWidgetPrivate::CItemWifiBoxWidgetPrivate(CItemWifiBoxWidget *dd)
    : q_ptr(dd)
{
    this->InitUI();
    this->InitData();
    this->InitConnect();
}

CItemWifiBoxWidget::CItemWifiBoxWidget(QWidget *parent)
    : QWidget(parent)
    , d_ptr(new CItemWifiBoxWidgetPrivate(this))
{

}

CItemWifiBoxWidget::~CItemWifiBoxWidget()
{

}

void CItemWifiBoxWidgetPrivate::InitUI()
{

    printf(">>>%s,%s,%d,\n",__FILE__,__func__,__LINE__);
    int width = QApplication::desktop()->screenGeometry().width();	
    m_pNameLabel = new QLabel;
    m_pWifiSwitchBox = new QCheckBox;

	m_pWifiSwitchBox->setFixedWidth(64);
    m_pRebootButton = new QPushButton(QObject::tr("rebootForTakeEffect"));//重启生效
	m_pRebootButton->show();   
    //m_pRebootButton->hide();	
    m_pIconLabel = new QLabel("....");
#if 1		
    QHBoxLayout *layout = new QHBoxLayout(q_func());

	
    layout->setMargin(0);
    layout->setSpacing(0);
    layout->addSpacing(25);


    layout->addWidget(m_pNameLabel);
    layout->addStretch(1);
	QHBoxLayout  *layoutReboot = new QHBoxLayout(q_func());
	QHBoxLayout  *layoutIcon = new QHBoxLayout(q_func());
	layoutReboot->addWidget(m_pRebootButton);
	layout->addLayout(layoutReboot);
	layoutIcon->addWidget(m_pIconLabel);
	layout->addLayout(layoutIcon);
    //layout->addWidget(m_pRebootButton);    
    //layout->addWidget(m_pIconLabel);	
    layout->addStretch(1);
	/*
	if (width==480)
	{
	  layout->addStretch(1);	
	  layout->addSpacing(20);//120,40，20	
	}
	*/
    layout->addWidget(m_pWifiSwitchBox);
    

	//if (width==480)
	//  layout->addSpacing(20);//120,40，20
    //else  
      layout->addSpacing(10);
#endif   
#if 0
	QGridLayout *gridLayout = new QGridLayout(q_func());
	gridLayout->addWidget(m_pRebootButton, 0, 0, 1, 1);
	gridLayout->addWidget(m_pIconLabel, 0, 1, 1, 1);
	gridLayout->addWidget(m_pWifiSwitchBox, 0, 2, 1, 1);
#endif 
}

void CItemWifiBoxWidgetPrivate::InitData()
{
    //m_pWifiSwitchBox->setFixedWidth(64);
    m_pWifiSwitchBox->setObjectName("WifiSwitchBox");
    q_func()->setObjectName("CItemWidget");

    if (m_pWifiSwitchBox->checkState())
    {
        m_pRebootButton->show();
        m_pIconLabel->show();
    }
    else 
    {
        m_pRebootButton->hide();    
        m_pIconLabel->hide();
    }
}

void CItemWifiBoxWidgetPrivate::InitConnect()
{
    QObject::connect(m_pWifiSwitchBox, &QCheckBox::stateChanged, q_func(), &CItemWifiBoxWidget::sigWifiSwitchState);
    QObject::connect(m_pWifiSwitchBox, &QCheckBox::stateChanged, [&] {
        if (m_pWifiSwitchBox->checkState())
        {
          m_pRebootButton->show();
          m_pIconLabel->show();    
          ReadConfig::GetInstance()->setNetwork_Manager_Mode(m_pWifiSwitchBox->checkState() ? 2 : 1);
		  //ReadConfig::GetInstance()->setNetwork_Manager_Mode(2);
          //QtConcurrent::run(ReadConfig::GetInstance(), &ReadConfig::setSaveConfig);	
		  sleep(1);	  
		  //ReadConfig::GetInstance()->setSaveConfig();
		  QtConcurrent::run(ReadConfig::GetInstance(), &ReadConfig::setSaveConfig);
          LogD(">>>%s,%s,%d,getNetwork_Manager_Mode=%d\n",__FILE__,__func__,__LINE__,ReadConfig::GetInstance()->getNetwork_Manager_Mode());          
		  sleep(1);
		  m_pIconLabel->setMovie(new QMovie(":/Images/progress.gif"));
          m_pIconLabel->movie()->start();      
        }
        else 
        {
          ReadConfig::GetInstance()->setNetwork_Manager_Mode(m_pWifiSwitchBox->checkState() ? 2 : 1);
          ReadConfig::GetInstance()->setSaveConfig();
          LogD(">>>%s,%s,%d,getNetwork_Manager_Mode=%d\n",__FILE__,__func__,__LINE__,ReadConfig::GetInstance()->getNetwork_Manager_Mode());     
          m_pRebootButton->hide();
          m_pIconLabel->hide();          
        }
    });

#if 1
    QObject::connect(m_pRebootButton, &QPushButton::clicked, [&] {
            if (m_pWifiSwitchBox->checkState())
            {
                 emit q_func()->sigRestartTakeEffect();
            }
        });
#endif 		
}

void CItemWifiBoxWidget::setData(const QString &Name)
{
    Q_D(CItemWifiBoxWidget);
    d->m_pNameLabel->setText(Name);
}

bool CItemWifiBoxWidget::getWifiState() const
{
#if 1    
    if (d_func()->m_pWifiSwitchBox->checkState())
       d_func()->m_pRebootButton->show();
    else 
       d_func()->m_pRebootButton->hide();
#endif        
    return d_func()->m_pWifiSwitchBox->checkState();
}

void CItemWifiBoxWidget::setWifiState(const bool &b)
{
    Q_D(CItemWifiBoxWidget);
    //d->m_pWifiSwitchBox->setChecked(true);
    d->m_pWifiSwitchBox->setChecked(b);
#if 1
    if (b)
       d->m_pRebootButton->show();
    else 
       d->m_pRebootButton->hide();
#endif        
}

void CItemWifiBoxWidget::setAddSpacing(const int spcaing)
{
    ((QHBoxLayout *)this->layout())->addSpacing(spcaing);
}
