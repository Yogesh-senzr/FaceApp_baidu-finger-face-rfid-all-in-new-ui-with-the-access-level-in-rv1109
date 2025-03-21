#include "VolumeFrm.h"
#include "sliderclick.h"

#include "PCIcore/Audio.h"
#include "Config/ReadConfig.h"

#include <QLabel>
#include <QHBoxLayout>
#include <QDebug>
#include <QEvent>
#include <QPushButton>

class VolumeFrmPrivate
{
    Q_DECLARE_PUBLIC(VolumeFrm)
public:
    VolumeFrmPrivate(VolumeFrm *dd);
private:
    void InitUI();
    void InitData();
    void InitConnect();
private:
    QLabel *m_pLeftHintLabel;
    SliderClick *m_pAdjustSlider;
    QLabel *m_pRightHintLabel;
    QPushButton *m_pAdd; //+号
    QPushButton *m_pReduce; //-号
private:
    VolumeFrm *const q_ptr;
};

VolumeFrmPrivate::VolumeFrmPrivate(VolumeFrm *dd)
    : q_ptr(dd)
{
    this->InitUI();
    this->InitData();
    this->InitConnect();
}

VolumeFrm::VolumeFrm(QWidget *parent)
    : QDialog(parent, Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint)
    , d_ptr(new VolumeFrmPrivate(this))
{

}

VolumeFrm::~VolumeFrm()
{

}

void VolumeFrmPrivate::InitUI()
{
    m_pLeftHintLabel = new QLabel("0");
    m_pReduce = new QPushButton; //-号
    m_pReduce->setText("一");
    m_pAdjustSlider = new SliderClick(Qt::Horizontal);
    m_pAdd = new QPushButton; //+号
    m_pAdd->setText("十");
    m_pRightHintLabel = new QLabel("100");

    QHBoxLayout *layout = new QHBoxLayout;
    layout->setMargin(0);
    layout->addStretch();
    layout->addWidget(new QLabel(QObject::tr("VolumeSetting")));//音量设置
    layout->addStretch();

    QHBoxLayout *layout1 = new QHBoxLayout;
    //layout1->addSpacing(20);
    layout1->addWidget(m_pLeftHintLabel);
    layout1->addWidget(m_pReduce);
    layout1->addWidget(m_pAdjustSlider);
    //layout1->addSpacing(8);
    layout1->addWidget(m_pAdd);
    layout1->addWidget(m_pRightHintLabel);
    //layout1->addSpacing(20);

    layout->itemAt(1)->widget()->setObjectName("DialogTitleLabel");
    m_pLeftHintLabel->setObjectName("VolumeFrmLabel");
    m_pRightHintLabel->setObjectName("VolumeFrmLabel");

    QVBoxLayout *vLayout = new QVBoxLayout(q_func());
    vLayout->setContentsMargins(0, 5, 0, 0);
    vLayout->addLayout(layout);
    vLayout->addLayout(layout1);
    vLayout->addStretch();
}

void VolumeFrmPrivate::InitData()
{
    q_func()->setAttribute(Qt::WA_QuitOnClose, false);
    q_func()->setObjectName("VolumeFrm");
    m_pAdjustSlider->setMinimum(0);
    m_pAdjustSlider->setMaximum(100);
}

void VolumeFrmPrivate::InitConnect()
{
    QObject::connect(m_pAdjustSlider, &QSlider::valueChanged, q_func(), &VolumeFrm::slotValueChanged);
    QObject::connect(m_pAdjustSlider, &QSlider::sliderReleased, q_func(), &VolumeFrm::slotSliderReleased);

    
    QObject::connect(m_pReduce, &QPushButton::clicked, [&] {       
            int mValue = m_pAdjustSlider->value();
            if (mValue>0) 
            {
                mValue--;
                printf(">>>%s,%s,%d,mValue=%d\n",__FILE__,__func__,__LINE__,mValue);
                m_pAdjustSlider->setValue(mValue);
            }
        });    
    QObject::connect(m_pAdd, &QPushButton::clicked, [&] {  
            int mValue = m_pAdjustSlider->value();     
            if (mValue<100) 
            {
                mValue++;
                printf(">>>%s,%s,%d,mValue=%d\n",__FILE__,__func__,__LINE__,mValue);
                m_pAdjustSlider->setValue(mValue);
            }
            });            
}

void VolumeFrm::setAdjustValue(const int &value)
{
    Q_D(VolumeFrm);
    d->m_pAdjustSlider->setValue(value);
}

int VolumeFrm::getAdjustValue()const
{
    return d_func()->m_pAdjustSlider->value();
}

void VolumeFrm::slotValueChanged(int value)
{
    Q_D(VolumeFrm);
    d->m_pLeftHintLabel->setText(QString::number(value));
    ReadConfig::GetInstance()->setVolume_Value(value);
}

void VolumeFrm::slotSliderReleased()
{
#ifdef Q_OS_LINUX
    YNH_LJX::Audio::Audio_PlayMedia_volume();
#endif
}

bool VolumeFrm::event(QEvent *event)
{
    if(event->type() == QEvent::User + 2)
    {
#ifdef Q_OS_LINUX
        YNH_LJX::Audio::Audio_PlayMedia_volume();
#endif
    }
    return QWidget::event(event);
}
#ifdef SCREENCAPTURE  //ScreenCapture 
void VolumeFrm::mouseDoubleClickEvent(QMouseEvent* event)
{
    grab().save(QString("/mnt/user/screenshot/%1.png").arg(this->metaObject()->className()),"png");    
}
#endif 