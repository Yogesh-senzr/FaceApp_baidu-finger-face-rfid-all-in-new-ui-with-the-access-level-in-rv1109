#include "TimesetFrm.h"
#include "Config/ReadConfig.h"
#include "NtpDate/NtpDateSync.h"
#include "ChangeDateTimeFrm.h"

#include <QLabel>
#include <QCheckBox>
#include <QComboBox>
#include <QPushButton>
#include <QHBoxLayout>
#include <QTimer>
#include <QDateTime>

class TimesetFrmPrivate
{
    Q_DECLARE_PUBLIC(TimesetFrm)
public:
    TimesetFrmPrivate(TimesetFrm *dd);
private:
    void InitUI();
    void InitData();
    void InitConnect();
private:
    void AutomaticSettingTime(const int);
    void AutomaticSettingRegion(const int);
private:
    QLabel *m_pShowCurTimerLabel;//显示当前时间和日期
    QCheckBox *m_pAutomaticSettingTimeBox;//自动设置时间
    QCheckBox *m_pAutomaticSettingRegionBox;//自动设置地区
    QComboBox *m_pTimeZonesBox;//时区
    QPushButton *m_pAlterBtn;//更改
private:
    QTimer *m_pReadTimer;

private:
    TimesetFrm *const q_ptr;
};

TimesetFrmPrivate::TimesetFrmPrivate(TimesetFrm *dd)
    : q_ptr(dd)
    , m_pReadTimer(new QTimer)
{
    this->InitUI();
    this->InitData();
    this->InitConnect();
}

TimesetFrm::TimesetFrm(QWidget *parent)
    : SettingBaseFrm(parent)
    , d_ptr(new TimesetFrmPrivate(this))
{
}

TimesetFrm::~TimesetFrm()
{
}

void TimesetFrmPrivate::InitUI()
{
    m_pShowCurTimerLabel = new QLabel;//显示当前时间和日期
    m_pAutomaticSettingTimeBox = new QCheckBox;//自动设置时间
    m_pAutomaticSettingRegionBox = new QCheckBox;//自动设置地区
    m_pTimeZonesBox = new QComboBox;//时区
    m_pAlterBtn = new QPushButton(QObject::tr("Modify"));//更改

    QVBoxLayout *vlayout = new QVBoxLayout(q_func());

    vlayout->setContentsMargins(30, 0, 0, 0);
    vlayout->addWidget(new QLabel(QObject::tr("CurrentDateSndTime")));//当前日期和时间
    vlayout->addSpacing(20);
    vlayout->addWidget(m_pShowCurTimerLabel);
    vlayout->addSpacing(20);

    vlayout->addWidget(new QLabel(QObject::tr("AutomaticSettingTime")));//自动设置时间
    vlayout->addWidget(m_pAutomaticSettingTimeBox);

    vlayout->addSpacing(20);
    vlayout->addWidget(new QLabel(QObject::tr("AutomaticSettingRegion")));//自动设置时区
    vlayout->addWidget(m_pAutomaticSettingRegionBox);

    vlayout->addSpacing(20);
    vlayout->addWidget(new QLabel(QObject::tr("ManualSettingRegion")));//手动设置日期和时间
    vlayout->addWidget(m_pAlterBtn);

    //    vlayout->addSpacing(40);
    //    vlayout->addWidget(new QLabel(QObject::tr("时区")));
    //    vlayout->addWidget(m_pTimeZonesBox);
    vlayout->addStretch();

}

void TimesetFrmPrivate::InitData()
{

    m_pAutomaticSettingTimeBox->setObjectName("TimesetFrmBox");
    m_pAutomaticSettingRegionBox->setObjectName("TimesetFrmBox");
    m_pAutomaticSettingTimeBox->setText(QObject::tr("Disable"));//自动设置时间,关
    m_pAutomaticSettingRegionBox->setText(QObject::tr("Disable"));//自动设置地区,关

    m_pAlterBtn->setFixedSize(121, 42);
    m_pTimeZonesBox->setFixedSize(500, 42);
}

void TimesetFrmPrivate::InitConnect()
{
    QObject::connect(m_pReadTimer, &QTimer::timeout, [&]{
        m_pShowCurTimerLabel->setText(QString("%1").arg(QDateTime::currentDateTime().toString(QObject::tr("yyyy年MM月dd日, hh:mm"))));
    });

    QObject::connect(m_pAutomaticSettingTimeBox, &QCheckBox::stateChanged, [&](int index){
        m_pAutomaticSettingTimeBox->setText(index ? QObject::tr("Enable") : QObject::tr("Disable"));//开,关
        this->m_pAlterBtn->setEnabled(index ? false : true);
        this->AutomaticSettingTime(index);
    });
    QObject::connect(m_pAutomaticSettingRegionBox, &QCheckBox::stateChanged, [&](int index){
        m_pAutomaticSettingRegionBox->setText(index ? QObject::tr("Enable") : QObject::tr("Disable"));
        this->AutomaticSettingRegion(index);
    });
    QObject::connect(m_pAlterBtn, &QPushButton::clicked, q_func(), &TimesetFrm::slotAlterBtn);
}

void TimesetFrmPrivate::AutomaticSettingTime(const int state)
{//自动设置时间
#if 0 //频繁开关时间设置,会卡顿,退出再进来,则会同步时间
    if(state)
    {
        NtpDateSync::GetInstance()->setSyncNtpDate();
    }
#endif     
}


void TimesetFrmPrivate::AutomaticSettingRegion(const int /*state*/)
{//自动设置时区

}

void TimesetFrm::setEnter()
{
    Q_D(TimesetFrm);
    d->m_pAutomaticSettingTimeBox->setChecked(ReadConfig::GetInstance()->getTimer_Manager_autoTime());
    d->m_pAutomaticSettingRegionBox->setChecked(ReadConfig::GetInstance()->getTimer_Manager_autoZone());

    //d->m_pShowCurTimerLabel->setText(QDateTime::currentDateTime().toString(QObject::tr("yyyy年MM月dd日, hh:mm")));
    int index = ReadConfig::GetInstance()->getLanguage_Mode();
   
    QLocale locale;
    if (index==0)
        locale = QLocale::Chinese;  
    if (index==1)
        locale = QLocale::English;     
    d->m_pShowCurTimerLabel->setText(locale.toString(QDateTime::currentDateTime(), QObject::tr("yyyy-MM-dd dddd hh:mm:ss")));    
    d->m_pReadTimer->start(1000);
}

void TimesetFrm::setLeaveEvent()
{
    Q_D(TimesetFrm);
    ReadConfig::GetInstance()->setTimer_Manager_autoTime(d->m_pAutomaticSettingTimeBox->checkState() ? 1 : 0);
    ReadConfig::GetInstance()->setTimer_Manager_autoZone(d->m_pAutomaticSettingRegionBox->checkState() ? 1 : 0);
    d->m_pReadTimer->stop();

    if(d->m_pAutomaticSettingTimeBox->checkState())
    {
        NtpDateSync::GetInstance()->setSyncNtpDate();
    }     
}

void TimesetFrm::slotAlterBtn()
{//弹出对话框让用户选择时间，然后读取在进行修改系统时钟
    ChangeDateTimeFrm dlg(this);
    dlg.exec();
}

#ifdef SCREENCAPTURE  //ScreenCapture 
void TimesetFrm::mouseDoubleClickEvent(QMouseEvent* event)
{
    grab().save(QString("/mnt/user/screenshot/%1.png").arg(this->metaObject()->className()),"png");    
}
#endif 