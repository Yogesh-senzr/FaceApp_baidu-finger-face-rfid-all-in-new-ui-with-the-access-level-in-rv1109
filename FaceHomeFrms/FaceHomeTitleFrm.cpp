#include "FaceHomeTitleFrm.h"
#include "Config/ReadConfig.h"
#include "Application/FaceApp.h"
#include <QLabel>
#include <QHBoxLayout>
#include <QDateTime>
#include <QStyleOption>
#include <QPainter>
#include <QTimer>

#include "BaseFace/BaseFaceManager.h"
#include "BaiduFace/BaiduFaceManager.h"
#include "Helper/myhelper.h"
#include "FaceMainFrm.h"
#include "MessageHandler/Log.h"

class FaceHomeTitleFrmPrivate
{
    Q_DECLARE_PUBLIC(FaceHomeTitleFrm)
public:
    FaceHomeTitleFrmPrivate(FaceHomeTitleFrm *dd);
private:
    void InitUI();
    void InitData();
    void InitConnect();

private:
    QLabel *m_pBackPngLabel;//背景图片
    QLabel *m_pClockLabel;
    QLabel *m_pNetPngLabel;
    QLabel *m_pHomeIconLabel;
	QTimer *m_timer;
    int mSec=0;//秒数
private:
    FaceHomeTitleFrm *m_FaceHomeTitleFrm;
private:
    FaceHomeTitleFrm *const q_ptr;
	
};

FaceHomeTitleFrmPrivate::FaceHomeTitleFrmPrivate(FaceHomeTitleFrm *dd)
    : q_ptr(dd)
{
    this->InitUI();
    this->InitData();
    this->InitConnect();
}

FaceHomeTitleFrm::FaceHomeTitleFrm(QWidget *parent)
    : QWidget(parent)
    , d_ptr(new FaceHomeTitleFrmPrivate(this))
{
}

FaceHomeTitleFrm::~FaceHomeTitleFrm()
{

}

void FaceHomeTitleFrmPrivate::InitUI()
{
    m_pClockLabel = new QLabel;
    m_pNetPngLabel = new QLabel;
    m_pBackPngLabel = new QLabel(q_func());
    // Add the home icon
    m_pHomeIconLabel = new QLabel(q_func());
    m_pHomeIconLabel->setPixmap(QPixmap(":/Images/Homeicon.png").scaled(32, 32, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    m_pHomeIconLabel->setGeometry(10, 10, 32, 32);


    m_pBackPngLabel->setGeometry(0, 0, DeskTopWidth, 42);
    m_pBackPngLabel->setPixmap(QPixmap(":/Images/FaceHomeTitleFrm_bg.png").scaled(DeskTopWidth,42, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
    QHBoxLayout *layout = new QHBoxLayout(q_func());
    layout->setMargin(0);
    layout->addWidget(m_pHomeIconLabel); 
    layout->addStretch();
    layout->addSpacing(20);
    layout->addWidget(m_pClockLabel);
    layout->addStretch();
    layout->addWidget(m_pNetPngLabel);
    layout->addSpacing(10);
	
	m_timer = new QTimer();
}

void FaceHomeTitleFrmPrivate::InitData()
{
    q_func()->setFixedHeight(42);
    q_func()->setObjectName("FaceHomeTitleFrm");
    m_pClockLabel->setObjectName("ClockLabel");

    /*
     * 	QLocale locale = QLocale::English;
    if (isc_setting::SStting::instance().getLanguage() == "zh")
    {
        locale = QLocale::Chinese;
    } else if (isc_setting::SStting::instance().getLanguage() == "en")
    {
        locale = QLocale::English;
    }
    mTopLabel->setText("\r\r\r\r\r\r\r\r\r\r\r\r" + locale.toString(QDateTime::currentDateTime(), tr(szTimerText)));
*/

	m_timer->start(1000); // 1秒单触发定时器
    int index = ReadConfig::GetInstance()->getLanguage_Mode();
   
    QLocale locale;
    if (index==0)
        locale = QLocale::English;  
    if (index==1)
        locale = QLocale::English;     
    m_pClockLabel->setText(locale.toString(QDateTime::currentDateTime(), QObject::tr("yyyy-MM-dd dddd hh:mm:ss")));
        
}

void FaceHomeTitleFrmPrivate::InitConnect()
{
	QObject::connect(m_timer, &QTimer::timeout,   [&]{

        int index = ReadConfig::GetInstance()->getLanguage_Mode();        

        QLocale locale;

        if (index==0)
           locale = QLocale::English;
        if (index==1)
           locale = QLocale::English;  
        m_pClockLabel->setText(locale.toString(QDateTime::currentDateTime(), QObject::tr("yyyy-MM-dd dddd hh:mm:ss")));
        m_pNetPngLabel->setVisible(true);
			
	});
}

void FaceHomeTitleFrm::setTitleText(const QString &text)
{
    Q_D(FaceHomeTitleFrm);
    d->m_pClockLabel->setText(text);

}

void FaceHomeTitleFrm::setLinkState(const bool &state, const int &type)
{
    Q_D(FaceHomeTitleFrm);

    if(state && (type == 1))
        d->m_pNetPngLabel->setPixmap(QPixmap(":/Images/ethernet_connect.png"));
    else if(state && (type == 2))
        d->m_pNetPngLabel->setPixmap(QPixmap(":/Images/wifi_connect.png"));
    else if(!state)d->m_pNetPngLabel->setPixmap(QPixmap(":/Images/ethernet_disconnect.png"));
    else d->m_pNetPngLabel->setPixmap(QPixmap(":/Images/ethernet_connect.png"));
}

void FaceHomeTitleFrm::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    QStyleOption opt;
    opt.init(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);
    QWidget::paintEvent(event);
		
}

